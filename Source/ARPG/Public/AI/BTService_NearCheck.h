// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_NearCheck.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UBTService_NearCheck : public UBTService
{
	GENERATED_BODY()
protected:

	UPROPERTY(EditAnywhere,Category="AI")
	FBlackboardKeySelector AttackRangeKey;

	UPROPERTY(EditAnywhere,Category="AI")
	float MaxAttackRange;

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
public:

	UBTService_NearCheck();
};
