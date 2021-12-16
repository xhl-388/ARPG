// Fill out your copyright notice in the Description page of Project Settings.


#include "ARPGPlayerState.h"

#include "Net/UnrealNetwork.h"

void AARPGPlayerState::OnRep_Credits(int32 OldCredits)
{
	OnCreditsChanged.Broadcast(this, Credits, Credits - OldCredits);
}

void AARPGPlayerState::OnRep_Lifes(int32 OldLifes)
{
	OnLifesChanged.Broadcast(this,Lifes,Lifes-OldLifes);
}

AARPGPlayerState::AARPGPlayerState()
{
	Lifes=3;
}


int32 AARPGPlayerState::GetCredits() const
{
	return Credits;
}

void AARPGPlayerState::AddCredits(int32 Delta)
{
	// Avoid user-error of adding a negative amount
	if (!ensure(Delta >= 0.0f))
	{
		return;
	}

	Credits += Delta;

	OnCreditsChanged.Broadcast(this, Credits, Delta);
}

bool AARPGPlayerState::RemoveCredits(int32 Delta)
{
	// Avoid user-error of adding a subtracting negative amount
	if (!ensure(Delta >= 0.0f))
	{
		return false;
	}

	if (Credits < Delta)
	{
		// Not enough credits available
		return false;
	}

	Credits -= Delta;

	OnCreditsChanged.Broadcast(this, Credits, -Delta);

	return true;
}

int32 AARPGPlayerState::GetLifes() const
{
	return Lifes;
}

void AARPGPlayerState::ReduceLife()
{
	Lifes--;

	OnLifesChanged.Broadcast(this,Lifes,-1);
}

void AARPGPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AARPGPlayerState, Credits);
}
