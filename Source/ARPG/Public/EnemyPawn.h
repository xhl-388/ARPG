// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Pawn.h"
#include "EnemyPawn.generated.h"

UCLASS()
class ARPG_API AEnemyPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AEnemyPawn();

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite,Category=Mesh)
	USkeletalMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere,Category=Attribute)
	UAttributeComponent* AttributeComponent;

	UPROPERTY(VisibleAnywhere,Category=Capsule)
	UCapsuleComponent* CapsuleComponent;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
