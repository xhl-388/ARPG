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

	// Give points to spend
	if (SpawnCreditCurve)
	{
		AvailableSpawnCredit += SpawnCreditCurve->GetFloatValue(GetWorld()->TimeSeconds);
	}

	if (CooldownBotSpawnUntil > GetWorld()->TimeSeconds)
	{
		// Still cooling down
		return;
	}

	GEngine->AddOnScreenDebugMessage(-1,2.f, FColor::Red,FString::Printf(TEXT("Available SpawnCredits: %f"), AvailableSpawnCredit));

	// Count alive bots before spawning
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
		// Reset before selecting new row
		SelectedMonsterRow = nullptr;

		TArray<FMonsterInfoRow*> Rows;
		MonsterTable->GetAllRows("", Rows);

		// Get total weight
		float TotalWeight = 0;
		for (FMonsterInfoRow* Entry : Rows)
		{
			TotalWeight += Entry->Weight;
		}

		// Random number within total random
		int32 RandomWeight = FMath::RandRange(0.0f, TotalWeight);

		//Reset
		TotalWeight = 0;

		// Get monster based on random weight
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
			// Too expensive to spawn, try again soon
			CooldownBotSpawnUntil = GetWorld()->TimeSeconds + CooldownTimeBetweenFailures;

			GEngine->AddOnScreenDebugMessage(-1,2.f, FColor::Red, FString::Printf(TEXT("Cooling down until: %f"), CooldownBotSpawnUntil));
			return;
		}
	}

	// Run EQS to find valid spawn location
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
			// Apply spawn cost
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

				// Grant special actions, buffs etc.
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
		if(ensure(PlayerPawn))
		{
			AARPGPlayerState* PlayerState=PlayerPawn->GetPlayerState<AARPGPlayerState>();
			check(PlayerState);
			PlayerState->ReduceLife();
			if(PlayerState->GetLifes()<=0)
			{
				UGameplayStatics::SetGamePaused(GetWorld(),true);
			}
			UAttributeComponent* AC=UAttributeComponent::GetAttributes(PlayerPawn);
			if(ensure(AC))
			{
				AC->ApplyHealthChange(nullptr,AC->GetHealthMax());
			}
		}
		AActor* PS=FindPlayerStart(Controller);
		if(ensure(PS))
		{
			GEngine->AddOnScreenDebugMessage(-1,2.f,FColor::Red,FString::Printf(TEXT(
				"PlayerStart:%s"),*PS->GetName()));
		}

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
	
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/BP_ARGPCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void AARPGGameModeBase::OnActorKilled(AActor* VictimActor, AActor* Killer)
{
	UE_LOG(LogTemp, Log, TEXT("OnActorKilled: Victim: %s, Killer: %s"), *GetNameSafe(VictimActor), *GetNameSafe(Killer));

	// Handle Player death
	AARPGCharacter* Player = Cast<AARPGCharacter>(VictimActor);
	if (Player)
	{
		FTimerHandle TimerHandle_RespawnDelay;
		FTimerDelegate Delegate;
		Delegate.BindUFunction(this, "RespawnPlayerElapsed", Player->GetController());
		
		float RespawnDelay = 2.0f;
		GetWorldTimerManager().SetTimer(TimerHandle_RespawnDelay, Delegate, RespawnDelay, false);

		// Store time if it was better than previous record
	}

	// Give Credits for kill
	APawn* KillerPawn = Cast<APawn>(Killer);
	// Don't credit kills of self
	if (KillerPawn && KillerPawn != VictimActor)
	{
		// Only Players will have a 'PlayerState' instance, bots have nullptr
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
	
	// Continuous timer to spawn in more bots.
	// Actual amount of bots and whether its allowed to spawn determined by spawn logic later in the chain...
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
