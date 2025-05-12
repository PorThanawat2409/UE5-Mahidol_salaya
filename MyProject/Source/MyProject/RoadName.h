// RoadName.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "RoadName.generated.h"

UCLASS()
class MYPROJECT_API ARoadName : public ATriggerBox
{
	GENERATED_BODY()

public:
	class ASimulatorWorld* SimWorld = nullptr;

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	FString GetRoadName();

	UFUNCTION(BlueprintCallable)
	void SetPreviousRoadInSimulator(FString NewRoadName);

	UFUNCTION(BlueprintCallable)
	FString GetPreviousRoadInSimulator();
};
