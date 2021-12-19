// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "EnvironmentQuery/EnvQueryInstanceBlueprintWrapper.h"
#include "GameFramework/GameModeBase.h"
#include "ARPGGameModeBase.generated.h"

USTRUCT(BlueprintType)
struct FMonsterInfoRow:public FTableRowBase
{
	GENERATED_BODY()
public:
	FMonsterInfoRow()
	{
		Weight = 1.0f;
		SpawnCost = 5.0f;
		KillReward = 20.0f;
	}

	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	FPrimaryAssetId MonsterId;

	/* Relative chance to pick this monster */
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	float Weight;

	/* Points required by gamemode to spawn this unit. */
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	float SpawnCost;

	/* Amount of credits awarded to killer of this unit.  */
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	float KillReward;
};

/**
 * 
 */
UCLASS()
class ARPG_API AARPGGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

protected:
	
	/* All available monsters */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	UDataTable* MonsterTable;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	UEnvQuery* SpawnBotQuery;

	/* Curve to grant credits to spend on spawning monsters */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	UCurveFloat* SpawnCreditCurve;
	
	/* Time to wait between failed attempts to spawn/buy monster to give some time to build up credits. */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float CooldownTimeBetweenFailures;

	FTimerHandle TimerHandle_SpawnBots;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float SpawnTimerInterval;

	// Read/write access as we could change this as our difficulty increases via Blueprint
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI")
	int32 CreditsPerKill;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> DeathHUDClass;
	
	UFUNCTION()
	void SpawnBotTimerElapsed();

	UFUNCTION()
	void OnBotSpawnQueryCompleted(UEnvQueryInstanceBlueprintWrapper* QueryInstance, EEnvQueryStatus::Type QueryStatus);

	void OnMonsterLoaded(FPrimaryAssetId LoadedId, FVector SpawnLocation);

	UFUNCTION()
	void RespawnPlayerElapsed(AController* Controller);

	// Points available to spend on spawning monsters
	float AvailableSpawnCredit;

	/* GameTime cooldown to give spawner some time to build up credits */
	float CooldownBotSpawnUntil;

	FMonsterInfoRow* SelectedMonsterRow;
	
public:
	AARPGGameModeBase();
	
	void OnActorKilled(AActor* VictimActor, AActor* Killer);

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	virtual void StartPlay() override;

	UFUNCTION(Exec)
	void KillAll();
};
