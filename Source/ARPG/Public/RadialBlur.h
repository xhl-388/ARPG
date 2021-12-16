// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/SceneCapture2D.h"
#include "GameFramework/Actor.h"
#include "RadialBlur.generated.h"

UCLASS()
class ARPG_API ARadialBlur : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARadialBlur();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY()
	UMaterial* RadialBlurMat;

	UPROPERTY()
	UMaterialInstanceDynamic* RadialBlurMID;

	UPROPERTY()
	APostProcessVolume* PostProcessVolumeActor;

	UPROPERTY()
	ASceneCapture2D* SceneCapture2D;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable,Category="PostProcess")
	void Trigger();

	UFUNCTION(BlueprintCallable,Category="PostProcess")
	void Shutdown();

	UFUNCTION(BlueprintCallable,Category="PostProcess")
	void SetWeight(float NewWeight);
};
