// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ControlArea.generated.h"

UCLASS()
class MYPROJECT_API AControlArea : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AControlArea();
	
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
		int Fee = 0;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
		FString Name = "";


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	

};
