	// Fill out your copyright notice in the Description page of Project Settings.


#include "MyChaosWVMovementComponent.h"


float FPIDController::ComputeNewInput(float Error, float Position)
{
    ErrorSum = Error + ErrorSum;

    float TermP = Error * Proportional;
    float TermI = ErrorSum * Integral;
    float TermD = Derivative * (LastPosition - Position);
    float Input = TermP + TermI + TermD;
    LastPosition = Position;

    return Input;
}


void UMyChaosWVMovementComponent::RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed)
{
	//UE_LOG(LogTemp, Warning, TEXT("UMyChaosWVMovementComponent was call"));
	CurrentSpeed = GetForwardSpeed() * 0.036f;

	Super::RequestDirectMove(MoveVelocity, bForceMaxSpeed);
	FVector VehicleLocation = GetOwner()->GetActorLocation();
	FVector Destination = VehicleLocation + MoveVelocity * GetWorld()->GetDeltaSeconds();
	FVector Distance = Destination - VehicleLocation;
	FVector DistanceToPoint = DestinationPoint - VehicleLocation;
	FVector InitialDistance = Destination - InitialLocation;

	FVector VehicleDirection = GetOwner()->GetActorForwardVector();

	//DrawDebugSphere(GetWorld(), Destination, 100.0f, 5, FColor::Black, false, 10.0f);

	float DistanceFloat = FVector::Dist(VehicleLocation, Destination);

	// DataTransferDelay(second) = interval time of request GPS data + respond time 
	// Default on ideal DataTransferDelay(second) = 1 
	float DataTransferDelay = 1.1; 
	//Slow down the car before reaching the destination.
	if (DistanceFloat <= 10000.0f) {
		float BreakValue = FMath::Clamp(1.0f - (DistanceFloat / 10000.0f), 0, 0.8f);
		float PercentSlow = FMath::Clamp((DistanceFloat / 10000.0f), 0.5f, 1.0f);

		MaxSpeed = (DistanceFloat) * 0.036f * PercentSlow;
		MaxSpeed = MaxSpeed / DataTransferDelay;
		SetBrakeInput(BreakValue);
	}
	else if (DistanceFloat > 200000.0f) {
		MaxSpeed = (DistanceFloat) * 0.036f * 1.20f;
		MaxSpeed = MaxSpeed / DataTransferDelay;
	}
	else {
		MaxSpeed = (DistanceFloat) * 0.036f;
		MaxSpeed = MaxSpeed / DataTransferDelay;
	}

	// Throttle controller
	float ForwardFactor = FVector::DotProduct(VehicleDirection, Distance.GetSafeNormal());
	float BackwardFactor = FVector::DotProduct(-VehicleDirection, Distance.GetSafeNormal());

	float Input;
	const float TURN_AROUND_FACTOR = bTurningAround ? 0.3f : 0.1f;

	ThrottleController.Proportional = 0.9f;
	ThrottleController.Integral = 0.0001f;
	ThrottleController.Derivative = 0.0f;

	UpdateMaxSpeed = sqrt(20.0f * 2.0f * (DistanceToPoint.Size() - 100.f) / 100.f);

	if (MaxSpeed < 2.0f)
	{

		if (UpdateMaxSpeed > LastMaxSpeed)
		{
			SpeedError = LastMaxSpeed - CurrentSpeed;
		}
		else
		{
			SpeedError = UpdateMaxSpeed - CurrentSpeed;
		}
	}
	else
	{
		SpeedError = MaxSpeed - CurrentSpeed;
		LastMaxSpeed = MaxSpeed;
	}

	// Calculate how much throttle need to make currentSpeed reach target speed
	Input = ThrottleController.ComputeNewInput(SpeedError, CurrentSpeed);
	float InputRaw = Input;
	if (FGenericPlatformMath::IsNaN(UpdateMaxSpeed))
		Input = -1;


	if (Input > 1)
		Input = 1;
	else if (Input < -1)
		Input = -1;
	// Move forward
	if (Input > 0.0f)
	{
		SetThrottleInput(Input);
		SetBrakeInput(0);

	}
	// Brake
	else
	{

		SetThrottleInput(Input);
		SetBrakeInput(-0.0005);
	}

	// Steering controller
	float InitialYaw = InitialDistance.Rotation().Yaw - InitialDirection.Rotation().Yaw;
	if (InitialYaw < -180)
	{
		InitialYaw += 360;
	}
	else if (InitialYaw > 180)
	{
		InitialYaw -= 360;
	}

	float CurrentYaw = Distance.Rotation().Yaw - VehicleDirection.Rotation().Yaw;
	if (CurrentYaw < -180)
	{
		CurrentYaw += 360;
	}
	else if (CurrentYaw > 180)
	{
		CurrentYaw -= 360;
	}

	float SteeringPosition = (-CurrentYaw + 180) / 180;
	float SteeringError = 1 - SteeringPosition;

	SteeringController.Proportional = 0.05f;
	SteeringController.Integral = 0.0f;
	SteeringController.Derivative = 0.0f;
	const float Steering = SteeringController.ComputeNewInput(CurrentYaw, CurrentYaw);

	//Slow down the car while entering a curve.
	if (((Steering <= -1.0) || (Steering >= 1.0)) && (CurrentSpeed>= 60.0f)) {
		SetBrakeInput(0.5);
	}
	else if (((Steering <= -1.0) || (Steering >= 1.0)) && (CurrentSpeed <= 50.0f)) {
		SetThrottleInput(0.5);
	}
	if (CurrentSpeed >= 100.0f) {
		SetBrakeInput(0.6);
	}

	if (CurrentSpeed > 0.01f)
	{
		SetSteeringInput(Steering);
	}
	else
	{
		SetSteeringInput(-Steering);
	}
	SetHandbrakeInput(false);


}

void UMyChaosWVMovementComponent::SetDestinationPoint(const FVector& NewDestinationPoint)
{
	DestinationPoint = NewDestinationPoint;
}

void UMyChaosWVMovementComponent::SetInitialLocation(const FVector& NewInitialLocation)
{
	InitialLocation = NewInitialLocation;
}
