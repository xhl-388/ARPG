// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/MyAIController.h"

void AMyAIController::BeginPlay()
{
	Super::BeginPlay();

	if(ensureMsgf(BehaviorTree,TEXT("Behavior Tree is nullptr!Please Assign BehaviorTree in your AI Controller.")))
	{
		RunBehaviorTree(BehaviorTree);
	}
}
