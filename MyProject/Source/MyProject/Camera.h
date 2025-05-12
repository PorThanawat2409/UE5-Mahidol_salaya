// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraActor.h"
#include "Camera.generated.h"

/**
 * 
 */
UCLASS()
class MYPROJECT_API ACamera : public ACameraActor
{
	GENERATED_BODY()

public:
	ACamera();
	void Activate();

	APlayerController* PlayerController;
	class ASimulatorWorld* SimWorld;

	UFUNCTION(BlueprintCallable)
	void ForwardTopViewCamera(float Value);
	UFUNCTION(BlueprintCallable)
	void RightTopViewCamera(float Value);
	UFUNCTION(BlueprintCallable)
	void YawTopViewCamera(float Value);
	UFUNCTION(BlueprintCallable)
	void PitchTopViewCamera(float Value);
	UFUNCTION(BlueprintCallable)
	void ZoomTopViewCamera(float Value);

	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
};
