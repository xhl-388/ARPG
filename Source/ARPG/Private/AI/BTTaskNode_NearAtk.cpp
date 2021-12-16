// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTaskNode_NearAtk.h"

#include "AIController.h"
#include "AI/AICharacter.h"

EBTNodeResult::Type UBTTaskNode_NearAtk::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* MyController=OwnerComp.GetAIOwner();

	GEngine->AddOnScreenDebugMessage(-1,2.f,FColor::Blue,TEXT("Spider Near Atk!"));
	
	if(ensure(MyController))
	{
		APawn* Pawn=MyController->GetPawn();
		check(Pawn);
		AAICharacter* Character=Cast<AAICharacter>(Pawn);
		if(ensure(Character))
		{
			Character->Attack(EAttackType::NAtk);
		}
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}
