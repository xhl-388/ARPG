// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionBase.h"
#include "ActionComponent.h"

#include "Net/UnrealNetwork.h"

UActionComponent* UActionBase::GetOwningComponent() const
{
	return ActionComp;
}

void UActionBase::OnRep_RepData()
{
	if (RepData.bIsRunning)
	{
		StartAction(RepData.Instigator);
	}
	else
	{
		StopAction(RepData.Instigator);
	}
}

void UActionBase::Initialize(UActionComponent* NewActionComp)
{
	 ActionComp=NewActionComp;
}

bool UActionBase::IsRunning() const
{
	return RepData.bIsRunning;
}

UWorld* UActionBase::GetWorld() const
{
	// Outer is set when creating action via NewObject<T>
	AActor* Actor = Cast<AActor>(GetOuter());
	if (Actor)
	{
		return Actor->GetWorld();
	}

	return nullptr;
}

void UActionBase::StopAction_Implementation(AActor* Instigator)
{
	UE_LOG(LogTemp, Log, TEXT("Stopped: %s"), *GetNameSafe(this));
	//LogOnScreen(this, FString::Printf(TEXT("Stopped: %s"), *ActionName.ToString()), FColor::White);

	//ensureAlways(bIsRunning);

	UActionComponent* Comp = GetOwningComponent();
	Comp->ActiveGameplayTags.RemoveTags(GrantsTags);

	RepData.bIsRunning = false;
	RepData.Instigator = Instigator;

	GetOwningComponent()->OnActionStopped.Broadcast(GetOwningComponent(), this);
}

void UActionBase::StartAction_Implementation(AActor* Instigator)
{
	UE_LOG(LogTemp, Log, TEXT("Started: %s"), *GetNameSafe(this));
	//LogOnScreen(this, FString::Printf(TEXT("Started: %s"), *ActionName.ToString()), FColor::Green);

	UActionComponent* Comp = GetOwningComponent();	
	Comp->ActiveGameplayTags.AppendTags(GrantsTags);

	RepData.bIsRunning = true;
	RepData.Instigator = Instigator;

	if (GetOwningComponent()->GetOwnerRole() == ROLE_Authority)
	{
		TimeStarted = GetWorld()->TimeSeconds;
	}

	GetOwningComponent()->OnActionStarted.Broadcast(GetOwningComponent(), this);
}

bool UActionBase::CanStart_Implementation(AActor* Instigator)
{
	if(IsRunning())
	{
		return false;
	}

	UActionComponent* Comp=GetOwningComponent();
	if(Comp->ActiveGameplayTags.HasAny(BlockedTags))
	{
		return false;
	}

	return true;
}


void UActionBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UActionBase, RepData);
	DOREPLIFETIME(UActionBase, TimeStarted);
	DOREPLIFETIME(UActionBase, ActionComp);
}