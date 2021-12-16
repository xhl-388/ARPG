// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ARPGPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API AARPGPlayerState : public APlayerState
{
	GENERATED_BODY()

	// Event Handler for Credits
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCreditsChanged, AARPGPlayerState*, PlayerState, int32, NewCredits, int32, Delta);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLifesChanged, AARPGPlayerState*, PlayerState, int32, NewLifes, int32, Delta);

	
	UPROPERTY(EditDefaultsOnly, ReplicatedUsing="OnRep_Credits", Category = "Credits")
	int32 Credits;

	UPROPERTY(EditDefaultsOnly,ReplicatedUsing="OnRep_Lifes",Category="Lifes")
	int32 Lifes;

	// OnRep_ can use a parameter containing the 'old value' of the variable it is bound to. Very useful in this case to figure out the 'delta'.
	UFUNCTION()
	void OnRep_Credits(int32 OldCredits);

	UFUNCTION()
	void OnRep_Lifes(int32 OldLifes);

public:

	AARPGPlayerState();
	
	UFUNCTION(BlueprintCallable, Category = "Credits")
	int32 GetCredits() const;

	UFUNCTION(BlueprintCallable, Category = "Credits")
	void AddCredits(int32 Delta);

	UFUNCTION(BlueprintCallable, Category = "Credits")
	bool RemoveCredits(int32 Delta);

	UFUNCTION(BlueprintCallable,Category="Lifes")
	int32 GetLifes() const;

	UFUNCTION(BlueprintCallable,Category="Lifes")
	void ReduceLife();

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCreditsChanged OnCreditsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLifesChanged OnLifesChanged;
};
