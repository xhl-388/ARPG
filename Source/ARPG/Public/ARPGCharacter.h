// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeComponent.h"
#include "GameFramework/Character.h"
#include "ARPGCharacter.generated.h"

UENUM(BlueprintType)
enum class EPlayerState:uint8
{
	IdleMove		UMETA(DisplayName="Free"),
	Atk1			UMETA(DisplayName="Attack_1") ,
	Atk2			UMETA(DisplayName="Attack_2"),
	Jump			UMETA(DisplayName="Jump"),
	Evade			UMETA(DisplayName="Evade"),
	NAtk			UMETA(DisplayName="NormalAttack")
};

UCLASS()
class ARPG_API AARPGCharacter : public ACharacter
{
	GENERATED_BODY()


	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Attribute,meta = (AllowPrivateAccess = "true"))
	class UAttributeComponent* AttributeComponent;
	
public:
	// Sets default values for this character's properties
	AARPGCharacter();

protected:
	UPROPERTY(VisibleAnywhere)
	EPlayerState MPlayerState;
	
	TMap<EPlayerState,FTimerHandle> CDTimerHandleMap;
	TMap<EPlayerState,FTimerHandle> StateTimerHandleMap;

	UPROPERTY(VisibleAnywhere)
	TMap<EPlayerState,float> CDMap;

	UPROPERTY(VisibleAnywhere)
	TMap<EPlayerState,bool> ReadyFlagMap;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	void ActionBase(EPlayerState TargetState);

	void Attack1();

	void Attack2();

	void NormalAttack();

	void Evade();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable)
	FORCEINLINE uint8 GetMPlayerState() const {return static_cast<uint8>(MPlayerState);}
	
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
