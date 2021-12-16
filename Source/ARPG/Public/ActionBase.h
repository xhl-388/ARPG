// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/NoExportTypes.h"
#include "ActionBase.generated.h"

class UWorld;
class UActionComponent;

USTRUCT()
struct FActionRepData
{
	GENERATED_BODY()

	UPROPERTY()
	bool bIsRunning;

	UPROPERTY()
	AActor* Instigator;
};

/**
 * 
 */
UCLASS(Blueprintable)
class ARPG_API UActionBase : public UObject
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="UI")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(Replicated)
	UActionComponent* ActionComp;

	UFUNCTION(BlueprintCallable,Category="Action")
	UActionComponent* GetOwningComponent() const;

	UPROPERTY(EditDefaultsOnly,Category="Tags")
	FGameplayTagContainer GrantsTags;

	UPROPERTY(EditDefaultsOnly,Category="Tags")
	FGameplayTagContainer BlockedTags;

	UPROPERTY(ReplicatedUsing="OnRep_RepData")
	FActionRepData RepData;

	UPROPERTY(Replicated)
	float TimeStarted;

	UFUNCTION()
	void OnRep_RepData();

public:

	void Initialize(UActionComponent* NewActionComp);

	UPROPERTY(EditDefaultsOnly,Category="Action")
	bool bAutoStart;

	UFUNCTION(BlueprintCallable,Category="Action")
	bool IsRunning() const;

	UFUNCTION(BlueprintNativeEvent,Category="Action")
	bool CanStart(AActor* Instigator);

	UFUNCTION(BlueprintNativeEvent,Category="Action")
	void StartAction(AActor* Instigator);

	UFUNCTION(BlueprintCallable,BlueprintNativeEvent,Category="Action")
	void StopAction(AActor* Instigator);

	UPROPERTY(EditDefaultsOnly,Category="Action")
	FName ActionName;

	UWorld* GetWorld() const override;

	bool IsSupportedForNetworking() const override
	{
		return true;
	}
};
