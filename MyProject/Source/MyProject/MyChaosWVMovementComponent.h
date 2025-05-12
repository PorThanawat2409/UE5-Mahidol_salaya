// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "MyChaosWVMovementComponent.generated.h"

USTRUCT()
struct FPIDController
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "PID")
    float Proportional = 1.0f;

    UPROPERTY(EditAnywhere, Category = "PID")
    float Integral = 1.0f;

    UPROPERTY(EditAnywhere, Category = "PID")
    float Derivative = 1.0f;

    UPROPERTY(EditAnywhere, Category = "PID")
    float ErrorMin = 0.0f;

    UPROPERTY(EditAnywhere, Category = "PID")
    float ErrorMax = 10.0f;

    float ErrorSum;
    float LastPosition;




    FPIDController() {}
    FPIDController(float P, float I, float D, float _ErrorMin, float _ErrorMax) : Proportional(P), Integral(I), Derivative(D),
        ErrorMin(_ErrorMin), ErrorMax(_ErrorMax), ErrorSum(0.0f), LastPosition(0.0f) {}
    float ComputeNewInput(float Error, float Position);
};

UCLASS()
class MYPROJECT_API  UMyChaosWVMovementComponent : public UChaosWheeledVehicleMovementComponent
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, Category = "PID")
    FPIDController ThrottleController;

    UPROPERTY(EditAnywhere, Category = "PID")
    FPIDController SteeringController;

public:

    FVector DestinationPoint;
    FVector InitialLocation;
    FVector InitialDirection;
    virtual void RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed) override;
    //virtual void StopActiveMovement() override;
    void SetDestinationPoint(const FVector& NewDestinationPoint);
    void SetInitialLocation(const FVector& NewInitialLocation);


private:

    
    float CurrentSpeed; 
    bool bTurningAround;
    float UpdateMaxSpeed;
    float MaxSpeed = 25.0f;
    float LastMaxSpeed = 0.0f;
    float SpeedError;
};