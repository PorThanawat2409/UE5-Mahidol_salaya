// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera.h"
#include "SimulatorWorld.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"


ACamera::ACamera()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = ETickingGroup::TG_PostPhysics;
	PrimaryActorTick.bTickEvenWhenPaused = true;
}

void ACamera::Activate()
{
	
}

void ACamera::ForwardTopViewCamera(float Value)
{
	FVector HeadVector;
	HeadVector = this->GetActorLocation();
	if (Value != 0)
	{
		HeadVector.X += Value * this->GetActorForwardVector().X * 30000.0f * GetWorld()->GetDeltaSeconds();
		HeadVector.Y += Value * this->GetActorForwardVector().Y * 30000.0f * GetWorld()->GetDeltaSeconds();
	}
	this->SetActorLocation(HeadVector);
}

void ACamera::RightTopViewCamera(float Value)
{
	FVector HeadVector;
	HeadVector = this->GetActorLocation();
	if (Value != 0)
	{
		HeadVector.X += Value * this->GetActorRightVector().X * 30000.0f * GetWorld()->GetDeltaSeconds();
		HeadVector.Y += Value * this->GetActorRightVector().Y * 30000.0f * GetWorld()->GetDeltaSeconds();
	}
	this->SetActorLocation(HeadVector);
}

void ACamera::YawTopViewCamera(float Value)
{
	FRotator HeadRotation;
	HeadRotation = this->GetActorRotation();
	if (Value != 0)
	{
		HeadRotation.Yaw += Value * 50 * GetWorld()->GetDeltaSeconds();
	}
	this->SetActorRotation(HeadRotation);

}

void ACamera::PitchTopViewCamera(float Value)
{
	FRotator HeadRotation;
	HeadRotation = this->GetActorRotation();
	if (Value != 0)
	{
		HeadRotation.Pitch += Value * 50 * GetWorld()->GetDeltaSeconds();
	}
	if (HeadRotation.Pitch < -65)
		HeadRotation.Pitch = -65;
	else if (HeadRotation.Pitch > -5)
		HeadRotation.Pitch = -5;

	this->SetActorRotation(HeadRotation);
}

void ACamera::ZoomTopViewCamera(float Value)
{
	FVector HeadVector;
	FRotator HeadRotation;
	if (Value != 0)
	{
		HeadRotation = this->GetActorRotation();
		HeadVector = this->GetActorLocation();
		FVector direction = FRotationMatrix(HeadRotation).GetScaledAxis(EAxis::X) * Value;
		FVector newPosition = HeadVector + direction * GetWorld()->GetDeltaSeconds();
		this->SetActorLocation(HeadVector + direction * GetWorld()->GetDeltaSeconds() * 70000.0f);
		//float currentGroundHeight = SimulatorWorld->GetEditorPlayMode()->GPSManager->mousePosition.Z + 120.0f;
		//if (HeadVector.Z >= currentGroundHeight)
		//{
		//	if (HeadVector.Z > ConfigCameraDataStruct.cameraCharacteristics.skyHeight)
		//		this->SetActorLocation(HeadVector + direction * GetWorld()->GetDeltaSeconds() * ConfigCameraDataStruct.cameraCharacteristics.zoomSpeedSky);
		//	else if (HeadVector.Z <= ConfigCameraDataStruct.cameraCharacteristics.skyHeight && HeadVector.Z >= ConfigCameraDataStruct.cameraCharacteristics.groundHeight)
		//		this->SetActorLocation(HeadVector + direction * GetWorld()->GetDeltaSeconds() * ConfigCameraDataStruct.cameraCharacteristics.zoomSpeedMid);
		//	else if (HeadVector.Z < ConfigCameraDataStruct.cameraCharacteristics.groundHeight)
		//		this->SetActorLocation(HeadVector + direction * GetWorld()->GetDeltaSeconds() * ConfigCameraDataStruct.cameraCharacteristics.zoomSpeedGround);
		//	else
		//		this->SetActorLocation(HeadVector + direction * GetWorld()->GetDeltaSeconds() * 5000.0f);
		//}

		//// If result of zoom up/down make camera position go lower than ground height, set to ground height
		//if (this->GetActorLocation().Z < currentGroundHeight)
		//{
		//	newPosition.Z = currentGroundHeight;
		//	this->SetActorLocation(newPosition);
		//}
	}
}


void ACamera::BeginPlay()
{
	Super::BeginPlay();
	
	SimWorld = Cast<ASimulatorWorld>(UGameplayStatics::GetActorOfClass(GetWorld(), ASimulatorWorld::StaticClass()));


}

void ACamera::Tick(float DeltaTime)
{
}
