// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionComponent.h"
#include "ActionBase.h"
#include "Engine/ActorChannel.h"

#include "Net/UnrealNetwork.h"


void UActionComponent::AddAction(AActor* Instigator, TSubclassOf<UActionBase> ActionClass)
{
	if(!ensure(ActionClass))
	{
		return;
	}

	if(!GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Client attempting to AddAction. [Class: %s]"), *GetNameSafe(ActionClass));
		return;
	}

	UActionBase* NewAction=NewObject<UActionBase>(GetOwner(),ActionClass);
	if(ensure(NewAction))
	{
		NewAction->Initialize(this);

		Actions.Add(NewAction);

		if(NewAction->bAutoStart&&ensure(NewAction->CanStart(Instigator)))
		{
			NewAction->StartAction(Instigator);
		}
	}
}

void UActionComponent::RemoveAction(UActionBase* ActionToRemove)
{
	if(!ensure(ActionToRemove&&!ActionToRemove->IsRunning()))
	{
		return ;
	}
	Actions.Remove(ActionToRemove);
}

UActionBase* UActionComponent::GetAction(TSubclassOf<UActionBase> ActionClass) const
{
	for(UActionBase* Action:Actions)
	{
		if(Action&&Action->IsA(ActionClass))
		{
			return Action;
		}
	}

	return nullptr;
}

bool UActionComponent::StartActionByName(AActor* Instigator, FName ActionName)
{

	for(UActionBase* Action:Actions)
	{
		if(Action&&Action->ActionName==ActionName)
		{
			if(!Action->CanStart(Instigator))
			{
				FString FailedMsg=FString::Printf(TEXT("Failed to run: %s"), *ActionName.ToString());
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FailedMsg);
				continue;
			}

			// Is Client?
			if (!GetOwner()->HasAuthority())
			{
				ServerStartAction(Instigator, ActionName);
			}

			// Bookmark for Unreal Insights
			TRACE_BOOKMARK(TEXT("StartAction::%s"), *GetNameSafe(Action));

			Action->StartAction(Instigator);
			return true;
		}
	}

	return false;
}

bool UActionComponent::StopActionByName(AActor* Instigator, FName ActionName)
{
	for (UActionBase* Action : Actions)
	{
		if (Action && Action->ActionName == ActionName)
		{
			if (Action->IsRunning())
			{
				// Is Client?
				if (!GetOwner()->HasAuthority())
				{
					ServerStopAction(Instigator, ActionName);
				}

				Action->StopAction(Instigator);
				return true;
			}
		}
	}

	return false;
}

// Sets default values for this component's properties
UActionComponent::UActionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
}

void UActionComponent::ServerStopAction_Implementation(AActor* Instigator, FName ActionName)
{
	StopActionByName(Instigator, ActionName);
}

void UActionComponent::ServerStartAction_Implementation(AActor* Instigator, FName ActionName)
{
	StartActionByName(Instigator,ActionName);
}


// Called when the game starts
void UActionComponent::BeginPlay()
{
	Super::BeginPlay();

	if(GetOwner()->HasAuthority())
	{
		for(TSubclassOf<UActionBase> ActionClass:DefaultActions)
		{
			AddAction(GetOwner(),ActionClass);
		}
	}
	
}

void UActionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	TArray<UActionBase*> ActionsCopy=Actions;
	for(UActionBase* Action:ActionsCopy)
	{
		if(Action&&Action->IsRunning())
		{
			Action->StopAction(GetOwner());
		}
	}
}


// Called every frame
void UActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UActionComponent::ReplicateSubobjects(class UActorChannel* Channel,class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	for (UActionBase* Action : Actions)
	{
		if (Action)
		{
			WroteSomething |= Channel->ReplicateSubobject(Action, *Bunch, *RepFlags);
		}
	}

	return WroteSomething;
}


void UActionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UActionComponent, Actions);
}

