// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionEffect.h"
#include "GameplayTagContainer.h"
#include "ProjectileBase.h"
#include "MagicProjectile.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API AMagicProjectile : public AProjectileBase
{
	GENERATED_BODY()
protected:
	
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float DamageAmount;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	FGameplayTag ParryTag;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	TSubclassOf<UActionEffect> BurningActionClass;

	UFUNCTION()
	void OnActorOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	virtual void PostInitializeComponents() override;

public:

	AMagicProjectile();
};
