// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTService_NearCheck.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

void UBTService_NearCheck::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	// Check distance between ai pawn and target actor
	UBlackboardComponent* BlackBoardComp = OwnerComp.GetBlackboardComponent();
	if (ensure(BlackBoardComp))
	{
		AActor* TargetActor = Cast<AActor>(BlackBoardComp->GetValueAsObject("TargetActor"));
		if (TargetActor)
		{
			AAIController* MyController = OwnerComp.GetAIOwner();

			APawn* AIPawn = MyController->GetPawn();
			if (ensure(AIPawn))
			{
				float DistanceTo = FVector::Distance(TargetActor->GetActorLocation(), AIPawn->GetActorLocation());

				bool bWithinRange = DistanceTo < MaxAttackRange;

				BlackBoardComp->SetValueAsBool(AttackRangeKey.SelectedKeyName, bWithinRange);
			}
		}
	}

}

UBTService_NearCheck::UBTService_NearCheck()
{
	MaxAttackRange=10.f;
}
