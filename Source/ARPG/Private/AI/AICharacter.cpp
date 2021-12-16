// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/AICharacter.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AAICharacter::AAICharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PawnSensingComp=CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));

	AttributeComp=CreateDefaultSubobject<UAttributeComponent>(TEXT("AttributeComp"));

	AutoPossessAI=EAutoPossessAI::PlacedInWorldOrSpawned;

	GetMesh()->SetGenerateOverlapEvents(true);

	TimeToHitParamName="TimeToHit";
	TargetActorKey="TargetActor";
}

// Called when the game starts or when spawned
void AAICharacter::BeginPlay()
{
	Super::BeginPlay();

	PawnSensingComp->OnSeePawn.AddDynamic(this,&AAICharacter::OnPawnSeen);
	AttributeComp->OnHealthChanged.AddDynamic(this,&AAICharacter::OnHealthChanged);
	
}

void AAICharacter::SetTargetActor(AActor* NewTarget)
{
	AAIController* AIC=Cast<AAIController>(GetController());
	if(AIC)
	{
		AIC->GetBlackboardComponent()->SetValueAsObject(TargetActorKey,NewTarget);
	}
	
}

AActor* AAICharacter::GetTargetActor() const
{
	AAIController* AIC=Cast<AAIController>(GetController());
	if(AIC)
	{
		return Cast<AActor>(AIC->GetBlackboardComponent()->GetValueAsObject(TargetActorKey));
	}
	return nullptr;
}

void AAICharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AAICharacter::OnHealthChanged(AActor* InstigatorActor, UAttributeComponent* OwningComp, float NewHealth,
	float Delta)
{
	GEngine->AddOnScreenDebugMessage(-1,2.f,FColor::Green,FString::Printf(TEXT("Enemy health changed!")));
	if(Delta<0.f)
	{
		if(InstigatorActor!=this)
		{
			SetTargetActor(InstigatorActor);
		}

		if(ActiveHealthBar==nullptr)
		{
			ActiveHealthBar=CreateWidget<UWorldUserWidget>(GetWorld(),HealthBarWidgetClass);
			if(ActiveHealthBar)
			{
				ActiveHealthBar->AttachedActor=this;
				ActiveHealthBar->AddToViewport();
			}
		}

		GetMesh()->SetScalarParameterValueOnMaterials(TimeToHitParamName,GetWorld()->TimeSeconds);

		//die
		if(NewHealth<=0.f)
		{
			//Stop BehaviorTree
			AAIController* AIC=Cast<AAIController>(GetController());
			if(AIC)
			{
				AIC->GetBrainComponent()->StopLogic("Killed");
			}

			GetMesh()->SetAllBodiesSimulatePhysics(true);
			GetMesh()->SetCollisionProfileName("Trigger");

			GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			GetCharacterMovement()->DisableMovement();

			//set lifespan
			SetLifeSpan(10.f);
		}
		
	}
}

void AAICharacter::OnPawnSeen(APawn* Pawn)
{
	if(GetTargetActor()!=Pawn)
	{
		SetTargetActor(Pawn);

		MulticastPawnSeen();
	}
	
}

void AAICharacter::MulticastPawnSeen_Implementation()
{
	UWorldUserWidget* NewWidget=CreateWidget<UWorldUserWidget>(GetWorld(),SpottedWidgetClass);
	if(NewWidget)
	{
		NewWidget->AttachedActor=this;
		NewWidget->AddToViewport(10);
	}
}

// Called every frame
void AAICharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AAICharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AAICharacter::Attack_Implementation(EAttackType AttackType)
{
}


