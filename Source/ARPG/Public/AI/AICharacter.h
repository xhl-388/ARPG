// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeComponent.h"
#include "WorldUserWidget.h"
#include "GameFramework/Character.h"
#include "Perception/PawnSensingComponent.h"
#include "AICharacter.generated.h"

UENUM(BlueprintType)
enum class EAttackType:uint8
{
	Charge,
	NAtk,
};

UCLASS()
class ARPG_API AAICharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAICharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UWorldUserWidget* ActiveHealthBar;
	
	UPROPERTY(EditDefaultsOnly,Category="UI")
	TSubclassOf<UUserWidget> HealthBarWidgetClass;

	// Widget to display when bot first sees a player
	UPROPERTY(EditDefaultsOnly,Category="UI")
	TSubclassOf<UUserWidget> SpottedWidgetClass;

	// material parameter for Hit Flashes 
	UPROPERTY(VisibleAnywhere,Category="Effects")
	FName TimeToHitParamName;

	// key for AI Blackboard 'TargetActor'
	UPROPERTY(VisibleAnywhere,Category="Effects")
	FName TargetActorKey;

	UFUNCTION(BlueprintCallable,Category="AI")
	void SetTargetActor(AActor* NewTarget);

	UFUNCTION(BlueprintCallable,Category="AI")
	AActor* GetTargetActor() const;

	virtual void PostInitializeComponents() override;

	UFUNCTION()
	void OnHealthChanged(AActor* InstigatorActor,UAttributeComponent* OwningComp,float NewHealth,float Delta);

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Components")
	UPawnSensingComponent* PawnSensingComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAttributeComponent* AttributeComp;

	UFUNCTION()
	void OnPawnSeen(APawn* Pawn);

	UFUNCTION(NetMulticast,Unreliable)
	void MulticastPawnSeen();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintNativeEvent)
	void Attack(EAttackType AttackType);
};
