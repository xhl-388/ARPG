// Copyright Epic Games, Inc. All Rights Reserved.


#include "ARPGGameModeBase.h"

#include "ActionBase.h"
#include "ActionComponent.h"
#include "ARPGCharacter.h"
#include "ARPGPlayerState.h"
#include "EnemyData.h"
#include "EngineUtils.h"
#include "AI/AICharacter.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "Kismet/GameplayStatics.h"

static TAutoConsoleVariable<bool> CVarSpawnBots(TEXT("su.SpawnBots"), true, TEXT("Enable spawning of bots via timer."), ECVF_Cheat);


void AARPGGameModeBase::SpawnBotTimerElapsed()
{
	if (!CVarSpawnBots.GetValueOnGameThread())
	{
		return;
	}

	// 根据浮点曲线在不同时间产生不同的点数
	if (SpawnCreditCurve)
	{
		AvailableSpawnCredit += SpawnCreditCurve->GetFloatValue(GetWorld()->TimeSeconds);
	}

	if (CooldownBotSpawnUntil > GetWorld()->TimeSeconds)
	{
		return;
	}

	GEngine->AddOnScreenDebugMessage(-1,2.f, FColor::Red,FString::Printf(TEXT("Available SpawnCredits: %f"), AvailableSpawnCredit));

	// 在生成怪物前判断怪物数量
	int32 NrOfAliveBots = 0;
	for (TActorIterator<AAICharacter> It(GetWorld()); It; ++It)
	{
		AAICharacter* Bot = *It;

		UAttributeComponent* AttributeComp = UAttributeComponent::GetAttributes(Bot);
		if (ensure(AttributeComp) && AttributeComp->IsAlive())
		{
			NrOfAliveBots++;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Found %i alive bots."), NrOfAliveBots);

	const float MaxBotCount = 40.0f;
	if (NrOfAliveBots >= MaxBotCount)
	{
		UE_LOG(LogTemp, Log, TEXT("At maximum bot capacity. Skipping bot spawn."));
		return;
	}

	if (MonsterTable)
	{
		// 重置
		SelectedMonsterRow = nullptr;

		TArray<FMonsterInfoRow*> Rows;
		MonsterTable->GetAllRows("", Rows);

		// 获得总生成权重
		float TotalWeight = 0;
		for (FMonsterInfoRow* Entry : Rows)
		{
			TotalWeight += Entry->Weight;
		}

		// 随机生成
		int32 RandomWeight = FMath::RandRange(0.0f, TotalWeight);
		
		TotalWeight = 0;

		// 根据weight作为概率生成怪物
		for (FMonsterInfoRow* Entry : Rows)
		{
			TotalWeight += Entry->Weight;

			if (RandomWeight <= TotalWeight)
			{
				SelectedMonsterRow = Entry;
				break;
			}
		}

		if (SelectedMonsterRow && SelectedMonsterRow->SpawnCost >= AvailableSpawnCredit)
		{
			// 选中的怪物没有点数去生成，等会儿再调用一次
			CooldownBotSpawnUntil = GetWorld()->TimeSeconds + CooldownTimeBetweenFailures;

			GEngine->AddOnScreenDebugMessage(-1,2.f, FColor::Red, FString::Printf(TEXT("Cooling down until: %f"), CooldownBotSpawnUntil));
			return;
		}
	}

	// 使用环境查询生成怪物
	UEnvQueryInstanceBlueprintWrapper* QueryInstance = UEnvQueryManager::RunEQSQuery(this, SpawnBotQuery, this, EEnvQueryRunMode::RandomBest5Pct, nullptr);
	if (ensure(QueryInstance))
	{
		QueryInstance->GetOnQueryFinishedEvent().AddDynamic(this, &AARPGGameModeBase::OnBotSpawnQueryCompleted);
	}
}

void AARPGGameModeBase::OnBotSpawnQueryCompleted(UEnvQueryInstanceBlueprintWrapper* QueryInstance,
	EEnvQueryStatus::Type QueryStatus)
{
	if (QueryStatus != EEnvQueryStatus::Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("Spawn bot EQS Query Failed!"));
		return;
	}

	TArray<FVector> Locations = QueryInstance->GetResultsAsLocations();
	if (Locations.IsValidIndex(0) && MonsterTable)
	{	
		if (UAssetManager* Manager = UAssetManager::GetIfValid())
		{
			// 应用生成怪物花费
			AvailableSpawnCredit -= SelectedMonsterRow->SpawnCost;

			FPrimaryAssetId MonsterId = SelectedMonsterRow->MonsterId;

			TArray<FName> Bundles;
			FStreamableDelegate Delegate = FStreamableDelegate::CreateUObject(this, &AARPGGameModeBase::OnMonsterLoaded, MonsterId, Locations[0]);
			Manager->LoadPrimaryAsset(MonsterId, Bundles, Delegate);
		}	
	}
}

void AARPGGameModeBase::OnMonsterLoaded(FPrimaryAssetId LoadedId, FVector SpawnLocation)
{
	UAssetManager* Manager = UAssetManager::GetIfValid();
	if (Manager)
	{
		UEnemyData* MonsterData = Cast<UEnemyData>(Manager->GetPrimaryAssetObject(LoadedId));
		if (MonsterData)
		{
			AActor* NewBot = GetWorld()->SpawnActor<AActor>(MonsterData->MonsterClass, SpawnLocation, FRotator::ZeroRotator);
			if (NewBot)
			{
				GEngine->AddOnScreenDebugMessage(-1,2.f,FColor::Red,FString::Printf(TEXT("Spawned enemy: %s (%s)"), *GetNameSafe(NewBot), *GetNameSafe(MonsterData)));

				// 加初始buff
				UActionComponent* ActionComp = Cast<UActionComponent>(NewBot->GetComponentByClass(UActionComponent::StaticClass()));
				if (ActionComp)
				{
					for (TSubclassOf<UActionBase> ActionClass : MonsterData->Actions)
					{
						ActionComp->AddAction(NewBot, ActionClass);
					}
				}
			}
		}
	}
}

void AARPGGameModeBase::RespawnPlayerElapsed(AController* Controller)
{
	if (ensure(Controller))
	{
		APawn* PlayerPawn=Controller->GetPawn();
		APlayerController* PlayerController=Cast<APlayerController>(Controller);
		if(ensure(PlayerPawn))
		{
			AARPGPlayerState* PlayerState=PlayerPawn->GetPlayerState<AARPGPlayerState>();
			check(PlayerState);
			PlayerState->ReduceLife();
			if(PlayerState->GetLifes()<=0)
			{
				UUserWidget* DeathHUD=CreateWidget<UUserWidget>(GetWorld(),DeathHUDClass.Get());
				DeathHUD->AddToViewport();
				PlayerController->SetShowMouseCursor(true);
				UGameplayStatics::SetGamePaused(GetWorld(),true);
			}
			UAttributeComponent* AC=UAttributeComponent::GetAttributes(PlayerPawn);
			if(ensure(AC))
			{
				AC->ApplyHealthChange(nullptr,AC->GetHealthMax());
			}
		}
		AActor* PS=FindPlayerStart(Controller);

		APawn* P=Controller->GetPawn();
		if(ensure(P))
		{
			P->SetActorLocation(PS->GetActorLocation());	
		}
		RestartPlayer(Controller);
	}
}

AARPGGameModeBase::AARPGGameModeBase()
{
	SpawnTimerInterval = 2.0f;
	CreditsPerKill = 20;
	CooldownTimeBetweenFailures = 8.0f;

	PlayerStateClass = AARPGPlayerState::StaticClass();
	
	// 设置玩家默认pawn为我的蓝图pawn
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/BP_ARGPCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void AARPGGameModeBase::OnActorKilled(AActor* VictimActor, AActor* Killer)
{
	UE_LOG(LogTemp, Log, TEXT("OnActorKilled: Victim: %s, Killer: %s"), *GetNameSafe(VictimActor), *GetNameSafe(Killer));

	// 处理玩家死亡事件
	AARPGCharacter* Player = Cast<AARPGCharacter>(VictimActor);
	if (Player)
	{
		FTimerHandle TimerHandle_RespawnDelay;
		FTimerDelegate Delegate;
		Delegate.BindUFunction(this, "RespawnPlayerElapsed", Player->GetController());
		
		float RespawnDelay = 2.0f;
		GetWorldTimerManager().SetTimer(TimerHandle_RespawnDelay, Delegate, RespawnDelay, false);
	}

	// 给予击杀积分
	APawn* KillerPawn = Cast<APawn>(Killer);
	// Don't credit kills of self
	if (KillerPawn && KillerPawn != VictimActor)
	{
		// 只有玩家拥有玩家状态
		AARPGPlayerState* PS = KillerPawn->GetPlayerState<AARPGPlayerState>();
		if (PS) 
		{
			PS->AddCredits(CreditsPerKill);
		}
	}
}

void AARPGGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
}

void AARPGGameModeBase::StartPlay()
{
	Super::StartPlay();

	if(MonsterTable==nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1,2.f,FColor::Red,TEXT("Monster table not found"));
	}
	if(SpawnBotQuery==nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1,2.f,FColor::Red,TEXT("Query not found"));
	}
	if(SpawnCreditCurve==nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1,2.f,FColor::Red,TEXT("Credit curve not found"));
	}
	
	// 持续生成怪物
	GetWorldTimerManager().SetTimer(TimerHandle_SpawnBots, this, &AARPGGameModeBase::SpawnBotTimerElapsed, SpawnTimerInterval, true);
	
}


void AARPGGameModeBase::KillAll()
{
	for (TActorIterator<AAICharacter> It(GetWorld()); It; ++It)
	{
		AAICharacter* Bot = *It;

		UAttributeComponent* AttributeComp = UAttributeComponent::GetAttributes(Bot);
		if (ensure(AttributeComp) && AttributeComp->IsAlive())
		{
			AttributeComp->Kill(this);
		}
	}
}
