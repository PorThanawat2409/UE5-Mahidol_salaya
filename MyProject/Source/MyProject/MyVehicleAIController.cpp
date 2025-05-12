// Fill out your copyright notice in the Description page of Project Settings.

#include "MyVehicleAIController.h"
#include "NavigationSystem.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"
#include "Camera/CameraActor.h"
#include "GameFramework/SpringArmComponent.h"
#include "MyVehicle.h"
#include "MyChaosWVMovementComponent.h"
#include "NavigationPath.h"

void AMyVehicleAIController::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Warning, TEXT("AI Controller BeginPlay"));
}

void AMyVehicleAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    UE_LOG(LogTemp, Warning, TEXT("AI Controller possessed Pawn: %s"), *InPawn->GetName());
}

void AMyVehicleAIController::MoveToTargetLocation(const FVector& Destination)
{
    EPathFollowingRequestResult::Type Result = MoveToLocation(Destination, AcceptanceRadius, bStopOnOverlap, bUsePathfinding, bProjectDestinationToNavigation, bCanStrafe, 0, bAllowPartialPaths);
    /*
    if (Result == EPathFollowingRequestResult::RequestSuccessful){
        DebugPath(Destination);
    }  
    */  
}
    
void AMyVehicleAIController::DebugPath(const FVector& Destination) {
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSys)
    {
        UNavigationPath* NavPath = NavSys->FindPathToLocationSynchronously(GetWorld(), GetPawn()->GetActorLocation(), Destination);
        if (NavPath && NavPath->IsValid())
        {
            for (int32 PointIndex = 0; PointIndex < NavPath->PathPoints.Num() - 1; ++PointIndex)
            {
                FVector PointA = NavPath->PathPoints[PointIndex];
                FVector PointB = NavPath->PathPoints[PointIndex + 1];
                DrawDebugLine(GetWorld(), PointA, PointB, FColor::Red, false, 10.0f, 0, 10.0f);
            }
        }
    }
}


bool AMyVehicleAIController::HasMultipleTurns(const FVector& Destination) const
{
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSys)
    {
        UNavigationPath* NavPath = NavSys->FindPathToLocationSynchronously(GetWorld(), GetPawn()->GetActorLocation(), Destination);
        if (NavPath && NavPath->IsValid())
        {
            int32 TurnCount = 0;

            for (int32 PointIndex = 1; PointIndex < NavPath->PathPoints.Num() - 1; ++PointIndex)
            {
                FVector PreviousDirection = (NavPath->PathPoints[PointIndex] - NavPath->PathPoints[PointIndex - 1]).GetSafeNormal();
                FVector CurrentDirection = (NavPath->PathPoints[PointIndex + 1] - NavPath->PathPoints[PointIndex]).GetSafeNormal();

                float TurnAngle = FMath::Acos(FVector::DotProduct(PreviousDirection, CurrentDirection));

                //TurnAngle over 30 degrees
                if (FMath::RadiansToDegrees(TurnAngle) > 30.0f)
                {
                    TurnCount++;

                    if (TurnCount >= 2)
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}
