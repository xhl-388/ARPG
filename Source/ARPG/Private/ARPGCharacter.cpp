// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPGCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values
AARPGCharacter::AARPGCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	MPlayerState=EPlayerState::IdleMove;

	CDTimerHandleMap.Add(EPlayerState::Atk1,FTimerHandle());
	CDTimerHandleMap.Add(EPlayerState::Atk2,FTimerHandle());
	CDTimerHandleMap.Add(EPlayerState::NAtk,FTimerHandle());
	CDTimerHandleMap.Add(EPlayerState::Evade,FTimerHandle());
	StateTimerHandleMap.Add(EPlayerState::Atk1,FTimerHandle());
	StateTimerHandleMap.Add(EPlayerState::Atk2,FTimerHandle());
	StateTimerHandleMap.Add(EPlayerState::NAtk,FTimerHandle());
	StateTimerHandleMap.Add(EPlayerState::Evade,FTimerHandle());
	CDMap.Add(EPlayerState::Atk1,3.f);
	CDMap.Add(EPlayerState::Atk2,3.f);
	CDMap.Add(EPlayerState::NAtk,0.7f);
	CDMap.Add(EPlayerState::Evade,1.3f);
	ReadyFlagMap.Add(EPlayerState::Atk1,true);
	ReadyFlagMap.Add(EPlayerState::Atk2,true);
	ReadyFlagMap.Add(EPlayerState::NAtk,true);
	ReadyFlagMap.Add(EPlayerState::Evade,true);
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	AttributeComponent=CreateDefaultSubobject<UAttributeComponent>(TEXT("AttributeComponent"));
}

// Called when the game starts or when spawned
void AARPGCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AARPGCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(MPlayerState==EPlayerState::IdleMove&&!GetMovementComponent()->IsMovingOnGround())
	{
		MPlayerState=EPlayerState::Jump;
	}else if(MPlayerState==EPlayerState::Jump&&GetMovementComponent()->IsMovingOnGround())
	{
		MPlayerState=EPlayerState::IdleMove;
	}
}

// Called to bind functionality to input
void AARPGCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("NAtk",IE_Pressed,this,&AARPGCharacter::NormalAttack);
	PlayerInputComponent->BindAction("Atk1",IE_Pressed,this,&AARPGCharacter::Attack1);
	PlayerInputComponent->BindAction("Atk2",IE_Pressed,this,&AARPGCharacter::Attack2);

	PlayerInputComponent->BindAction("Evade",IE_Pressed,this,&AARPGCharacter::Evade);
	
	PlayerInputComponent->BindAxis("MoveY", this, &AARPGCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveX", this, &AARPGCharacter::MoveRight);
	
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

}

void AARPGCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AARPGCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AARPGCharacter::ActionBase(EPlayerState TargetState)
{
	if(ReadyFlagMap[TargetState]&&MPlayerState==EPlayerState::IdleMove)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, StaticEnum<EPlayerState>()->GetNameStringByValue(static_cast<uint8>(TargetState)));

		ReadyFlagMap[TargetState]=false;
		MPlayerState=TargetState;
		
		FTimerDelegate ResetCD;
		ResetCD.BindLambda([this,TargetState]
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, FString::Printf(TEXT("%s Cooldown"),
				*StaticEnum<EPlayerState>()->GetNameStringByValue(static_cast<uint8>(TargetState))));
			this->ReadyFlagMap[TargetState]=true;
		});
		GetWorldTimerManager().SetTimer(CDTimerHandleMap[TargetState],ResetCD,CDMap[TargetState],false);
		
		FTimerDelegate ResetAtkState;
		ResetAtkState.BindLambda([this]
		{
			this->MPlayerState=EPlayerState::IdleMove;
		});
		GetWorldTimerManager().SetTimer(StateTimerHandleMap[TargetState],ResetAtkState,0.01f,false);
	}
}

void AARPGCharacter::Attack1()
{
	ActionBase(EPlayerState::Atk1);
}

void AARPGCharacter::Attack2()
{
	ActionBase(EPlayerState::Atk2);
}

void AARPGCharacter::NormalAttack()
{
	ActionBase(EPlayerState::NAtk);
}

void AARPGCharacter::Evade()
{
	ActionBase(EPlayerState::Evade);
}
