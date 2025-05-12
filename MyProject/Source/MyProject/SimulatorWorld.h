// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GeoReferencingSystem.h"
#include "WheeledVehiclePawn.h"
#include "UnrealYAML/Public/Parsing.h"
#include "UnrealYAML/Public/Node.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Http.h"
#include "APIService.h"
#include "MyVehicle.h"
#include "WBP_Notification.h"
#include "SimulatorWorld.generated.h"

#define LANE_DETECTION_TRACE_CHANNEL ECC_GameTraceChannel1

USTRUCT(BlueprintType)
struct FControlAreaData
{
	GENERATED_BODY()
	FString status = "";
};

USTRUCT(BlueprintType)
struct FCongestionSructure : public FTableRowBase
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString congestionName = "";
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float congestionFee = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float positionX = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float positionY = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float positionZ = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float rotationX = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float rotationY = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float rotationZ = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float scaleX = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float scaleY = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float scaleZ = 0;
};

USTRUCT(BlueprintType)
struct FMapData : public FTableRowBase
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<struct FCongestionSructure> congestionDatalist;
};

USTRUCT(BlueprintType)
struct FMapJson : public FTableRowBase
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FMapData mapDatalist;
};

enum EWheelID
{
	FrontLeft = 0,
	FrontRight
};

UCLASS()
class MYPROJECT_API ASimulatorWorld : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASimulatorWorld();
	TArray<AActor* > Players;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
		FString CurrentTargetLatitude = "";
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
		FString CurrentTargetLongtitude = "";
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
		class AGeoReferencingSystem* geo;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
		bool IsCompletedGetGPS = false;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
		bool IsStartGetGPS = false;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
		bool IsStartPostControlAreaStatus = false;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
		bool IsEnableMoveToGPSLocation = false;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
		float TempAltitude = 10;

	// AWheeledVehiclePawn => AMyVehicle
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
		class AMyVehicle* Player;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
		class AWheeledVehiclePawn* PlayerTemp;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
		float Timer = 0.f;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
		float GetGPSDelay = 0.5f;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
		FString GoogleSheetAPIURL;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
		bool IsControlAreaMode = false;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
		APlayerController* PlayerController = nullptr;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
		class ACamera* TopViewCamera = nullptr;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
		bool IsEditorMode = false;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
		float VehicleWidth;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
		float TimeNotMoving = 0.0f;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
		FVector LastPlayerLocation = FVector::ZeroVector;
	
	////custom get api
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	AAPIService* APIService = nullptr;
	//custom vehicle
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	AMyVehicle* MyVehicle = nullptr;
	 
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UWBP_Notification> NotificationWidgetClass = nullptr;

	UPROPERTY(Transient)
	AActor* PlacementPreview = nullptr;
	bool IsPlacedActorEditing = false;
	bool IsCancleEditing = false;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	bool IsSpawndEditedActor = true;
	FVector BeforeEditingPosition;
	FRotator BeforeEditingRotation;
	FVector  BeforeEditingScale;


	float DistanceToLastTarget = 0.0f;
	bool IsFirstTimeMove = true;
	bool IsFirstTimeGPS = true;
	bool IsFirstTarget = true;
	FVector PreviousActorLocation = FVector(0, 0, 0);
	FVector BeforePreviousActorLocation = FVector(0, 0, 0);
	FVector TargetLocation = FVector(0, 0, 0);
	FRotator TargetRotation = FRotator(0, 0, 0);
	FVector TempTargetLocation = FVector(0, 0, 0);
	FRotator TempTargetRotation = FRotator(0, 0, 0);
	TArray<FVector> ListTargetLocation;
	TArray<FRotator> ListTargetRotation;
	TArray<FVector> FirstMoveList;
	TArray<FVector> GPSList;
	float BufferLength = 1.0f;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);
	FHttpModule* Http = nullptr;
	class UMyGameInstance* GameInstance = nullptr;
	FString const PATH_WBP_ControlAreaSetting_C = FString(TEXT("/Game/VirtualWorld/Widget/WBP_ControlAreaSetting.WBP_ControlAreaSetting_C"));
	
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void MovePlayerToGPSLocation(FString latitude, FString longtitude, float DeltaTime);
	void MoveLocation(FVector playerLocation);
	FVector GetCTargetLocation();
	void OnGetTimeResponse(FHttpRequestPtr req, FHttpResponsePtr res, bool isConnectSuccessfull);
	void OnPostTimeResponse(FHttpRequestPtr req, FHttpResponsePtr res,  bool isConnectSuccessfull);
	void LoadMap();

	FName LeftFrontWheelName = "WheelFL";
	FName RightFrontWheelName = "WheelFR";
	FName LeftBackWheelName = "WheelRL";
	FName RightBackWheelName = "WheelRR";

	//float FindDistanceToLane(EWheelID wheelID, float direction);
	UFUNCTION(BlueprintCallable)
		void EnablePhysic();
	UFUNCTION(BlueprintCallable)
		void DisablePhysic();
	UFUNCTION(BlueprintCallable)
		void SaveMap();
	UFUNCTION(BlueprintCallable)
		bool CheckPlacementOverlap(AActor* previewActor);
	UFUNCTION(BlueprintCallable)
		bool CheckPlacementOverlapRoad(AActor* previewActor);
	UFUNCTION(BlueprintCallable)
		void StopControlAreaPlacement();
	UFUNCTION(BlueprintCallable)
		void GetGPSFromServer();
	UFUNCTION(BlueprintCallable)
		void PostControlAreaStatusToServer(bool InArea, int fee, FString name);
	UFUNCTION(BlueprintCallable)
		void RunMovePlayerToGPSLocation(float DeltaTime);
	UFUNCTION(BlueprintCallable)
		void StopMovePlayerToGPSLocation();
	
	UFUNCTION(BlueprintCallable)
		void HideAllWidgetFromInput(UUserWidget* widgetInput);
	UFUNCTION(BlueprintCallable)
		void UnHideAllWidget();

	UFUNCTION(BlueprintCallable)
		void EnableTopViewCameraMode();
	UFUNCTION(BlueprintCallable)
		void DisableTopViewCameraMode();

	UFUNCTION(BlueprintCallable)
		void SpawnPlacementPreviewActor(TSubclassOf<AActor> ObjectClass);
	void UpdatePlacementPreviewPosition();
	AActor* SpawnNewObjectAtPlace();
	UFUNCTION(BlueprintCallable)
	void YawRotatePlacementPreview(float Value);
	UFUNCTION(BlueprintCallable)
	void PitchRotatePlacementPreview(float Value);
	UFUNCTION(BlueprintCallable)
	void ScaleWidthPlacementPreview(float Value);
	UFUNCTION(BlueprintCallable)
	void ScaleLengthPlacementPreview(float Value);

	UFUNCTION(BlueprintCallable)
	void OnMouseLeftClick();
	UFUNCTION(BlueprintCallable)
	void OnESCKey();

	UFUNCTION(BlueprintCallable)
	void OnClickControlArea(AActor* ControlArea);

	void CreateFirstMoveList(FVector& NewplayerLocation);
	void CreateGPSList(FVector& NewplayerLocation);

	UFUNCTION(BlueprintCallable)
	FVector GetCloseLocationOnNavMesh(FVector& Destination);
	bool CheckLocationOnNavMesh(FVector& Destination);
	float GetPathLengthToDestination(const FVector& Destination, const FVector* DefaultStartLocation = nullptr);
	bool HandleGPSBehindVehicle(FVector& Destination);
	FVector HandleGPSWrongRoad(FVector& Destination, const FVector* ForwardVector = nullptr);
	bool IsWrongRoad();
	bool DestinationIsOnLeftSideOfVehicle(FVector& Destination);
	void ResetAIMove();

	FString PreviousRoad;
};
