// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MyChaosWVMovementComponent.h"
//#include <Camera/CameraComponent.h>
#include "MyVehicleAIController.generated.h"

UCLASS()
class MYPROJECT_API AMyVehicleAIController : public AAIController
{
    GENERATED_BODY()
public:

    virtual void BeginPlay() override;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float AcceptanceRadius = 50.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bStopOnOverlap = true;
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bUsePathfinding = true;
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bProjectDestinationToNavigation = true;
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bCanStrafe = true;
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bAllowPartialPaths = true;

    void MoveToTargetLocation(const FVector& Destination);
    void DebugPath(const FVector& Destination);
    bool HasMultipleTurns(const FVector& Destination) const;

    virtual void OnPossess(APawn* InPawn) override;
    
};