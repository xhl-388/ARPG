// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTaskNode_Charge.h"

#include "AIController.h"
#include "AI/AICharacter.h"

EBTNodeResult::Type UBTTaskNode_Charge::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* MyController=OwnerComp.GetAIOwner();
	if(ensure(MyController))
	{
		APawn* Pawn=MyController->GetPawn();
		check(Pawn);
		AAICharacter* Character=Cast<AAICharacter>(Pawn);
		if(ensure(Character))
		{
			Character->Attack(EAttackType::Charge);
		}
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}
