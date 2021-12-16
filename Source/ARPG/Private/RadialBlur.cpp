// Fill out your copyright notice in the Description page of Project Settings.


#include "RadialBlur.h"

#include "EngineUtils.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/Character.h"
#include "Particles/ParticleEmitter.h"

// Sets default values
ARadialBlur::ARadialBlur()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARadialBlur::BeginPlay()
{
	Super::BeginPlay();

	// load material and create mid
	RadialBlurMat = LoadObject<UMaterial>(GetTransientPackage(), TEXT("Material'/Game/Materials/M_RadialBlur.M_RadialBlur'"));
	if (RadialBlurMat != nullptr)
	{
		RadialBlurMID = UMaterialInstanceDynamic::Create(RadialBlurMat, this, FName("RadialBlurMID"));
	}

	// find post process volume
	int32 num = GetWorld()->PostProcessVolumes.Num();
	if (num > 0)
	{
		PostProcessVolumeActor = Cast<APostProcessVolume>(GetWorld()->PostProcessVolumes[0]);
		PostProcessVolumeActor->bEnabled = true;
		PostProcessVolumeActor->BlendWeight = 1.0f;
		PostProcessVolumeActor->bUnbound = true;
	}
	
}

// Called every frame
void ARadialBlur::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

void ARadialBlur::Trigger()
{
	if (PostProcessVolumeActor != nullptr)
	{
		PostProcessVolumeActor->AddOrUpdateBlendable(RadialBlurMID, 1);
	}
}

void ARadialBlur::Shutdown()
{
	if (PostProcessVolumeActor != nullptr)
	{
		PostProcessVolumeActor->AddOrUpdateBlendable(RadialBlurMID, 0);
	}
}

void ARadialBlur::SetWeight(float NewWeight)
{
	NewWeight=FMath::Clamp(NewWeight,0.f,1.f);
	if (PostProcessVolumeActor != nullptr)
	{
		PostProcessVolumeActor->AddOrUpdateBlendable(RadialBlurMID, NewWeight);
	}
}
