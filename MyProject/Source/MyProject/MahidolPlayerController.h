// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyVehicle.h"
#include "APIService.h"
#include "GeoReferencingSystem.h"
#include "InputMappingContext.h"
#include "MahidolPlayerController.generated.h"

UCLASS()
class MYPROJECT_API AMahidolPlayerController : public APlayerController 
{
	GENERATED_BODY()
protected:
	virtual void SetupInputComponent() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void BeginPlay() override;

	AMahidolPlayerController();

	UFUNCTION(BlueprintCallable)
	void SwitchActor();

	UFUNCTION(BlueprintCallable)
	FVector GetCloseLocationOnNavMesh(FVector& Destination);

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	APawn* PlayerActor = nullptr; 

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	AAPIService* APIService = nullptr;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	AMyVehicle* VehicleActor = nullptr;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	class AGeoReferencingSystem* geo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite) 
	UInputMappingContext* InputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> WBP_MainUI;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UUserWidget* WidgetInstance;

	FVector PlayerLocation;
	FRotator PlayerRotation;
	FVector VehicleLocation;
	FRotator VehicleRotation;
	
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	FString TargetLatitude = "";

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	FString TargetLongtitude = "";

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	bool IsEnableMoveToGPS = false;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	bool IsPlayerActor = true;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	bool IsFirstTimeMove = true;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	bool IsCompletedGetGPS = false;

	UPROPERTY()
	class UUserWidget* MinimapWidget;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> MinimapWidgetClass;

	void ToggleMinimap();
	void MoveActorToLocation();

	FVector GetDestination();
	
	UFUNCTION(BlueprintCallable)
	void PostStatusToServer(bool InArea, int fee, FString name);

	float DistanceToLastTarget = 0.0f;
};
