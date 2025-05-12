// RoadName.cpp
#include "RoadName.h"
#include "GameFramework/Actor.h"
#include "SimulatorWorld.h"
#include "Engine/Engine.h"
#include "Engine.h"

void ARoadName::BeginPlay()
{
	Super::BeginPlay();
    SimWorld = Cast<ASimulatorWorld>(UGameplayStatics::GetActorOfClass(GetWorld(), ASimulatorWorld::StaticClass()));
}

FString ARoadName::GetRoadName()
{
#if WITH_EDITOR
    FString Name = GetActorLabel();
    return Name;
#else
    FString Name = GetName();
    return Name;
#endif
}

void ARoadName::SetPreviousRoadInSimulator(FString NewRoadName)
{
    if (SimWorld)
    {
        SimWorld->PreviousRoad = NewRoadName;

    }
}

FString ARoadName::GetPreviousRoadInSimulator()
{
    if(SimWorld)
    {
        return SimWorld->PreviousRoad;

    }
    else {
        UE_LOG(LogTemp, Warning, TEXT("Not found SimWorld "));
    }
    return FString();
}