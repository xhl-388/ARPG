// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTaskNode_HealSelf.h"

#include "AIController.h"
#include "AttributeComponent.h"

EBTNodeResult::Type UBTTaskNode_HealSelf::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	APawn* MyPawn = Cast<APawn>(OwnerComp.GetAIOwner()->GetPawn());
	if (MyPawn == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	UAttributeComponent* AttributeComp = UAttributeComponent::GetAttributes(MyPawn);
	if (ensure(AttributeComp))
	{
		AttributeComp->ApplyHealthChange(MyPawn, AttributeComp->GetHealthMax());
	}

	return EBTNodeResult::Succeeded;
}
