// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicProjectile.h"

#include "ActionComponent.h"
#include "ARPGFunctionLibrary.h"

void AMagicProjectile::OnActorOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != GetInstigator())
	{
		//static FGameplayTag Tag = FGameplayTag::RequestGameplayTag("Status.Parrying");

		// Parry Ability (GameplayTag Example)
		UActionComponent* ActionComp = Cast<UActionComponent>(OtherActor->GetComponentByClass(UActionComponent::StaticClass()));
		if (ActionComp && ActionComp->ActiveGameplayTags.HasTag(ParryTag))
		{
			MoveComp->Velocity = -MoveComp->Velocity;

			SetInstigator(Cast<APawn>(OtherActor));
			return;
		}

		// Apply Damage & Impulse
		if (UARPGFunctionLibrary::ApplyDirectionalDamage(GetInstigator(), OtherActor, DamageAmount, SweepResult))
		{
			// We only explode if the target can be damaged, it ignores anything it Overlaps that it cannot Damage (it requires an AttributeComponent on the target)
			Explode();

			if (ActionComp && BurningActionClass && HasAuthority())
			{
				ActionComp->AddAction(GetInstigator(), BurningActionClass);
			}
		}
	}
}

void AMagicProjectile::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AMagicProjectile::OnActorOverlap);
}

AMagicProjectile::AMagicProjectile()
{
	SphereComp->SetSphereRadius(20.0f);
	InitialLifeSpan = 10.0f;
	DamageAmount = 20.0f;
}
