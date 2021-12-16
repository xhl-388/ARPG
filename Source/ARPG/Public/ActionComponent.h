// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "ActionComponent.generated.h"

class UActionBase;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ARPG_API UActionComponent : public UActorComponent
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActionStateChanged,UActionComponent*,OwningComp,UActionBase*,Action);
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Tags")
	FGameplayTagContainer ActiveGameplayTags;

	UFUNCTION(BlueprintCallable,Category="Actions")
	void AddAction(AActor* Instigator,TSubclassOf<UActionBase> ActionClass);

	UFUNCTION(BlueprintCallable,Category="Actions")
	void RemoveAction(UActionBase* ActionToRemove);

	UFUNCTION(BlueprintCallable,Category="Actions")
	UActionBase* GetAction(TSubclassOf<UActionBase> ActionClass) const;

	UFUNCTION(BlueprintCallable,Category="Actions")
	bool StartActionByName(AActor* Instigator,FName ActionName);

	UFUNCTION(BlueprintCallable,Category="Actions")
	bool StopActionByName(AActor* Instigator,FName ActionName);
	
	// Sets default values for this component's properties
	UActionComponent();

protected:
	UFUNCTION(Server,Reliable)
	void ServerStartAction(AActor* Instigator,FName ActionName);

	UFUNCTION(Server,Reliable)
	void ServerStopAction(AActor* Instigator,FName ActionName);

	UPROPERTY(EditAnywhere,Category="Actions")
	TArray<TSubclassOf<UActionBase>> DefaultActions;

	UPROPERTY(BlueprintReadOnly,Replicated)
	TArray<UActionBase*> Actions; 
	
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UPROPERTY(BlueprintAssignable)
	FOnActionStateChanged OnActionStarted;

	UPROPERTY(BlueprintAssignable)
	FOnActionStateChanged OnActionStopped;

	virtual bool ReplicateSubobjects(class UActorChannel* Channel,class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
