// Fill out your copyright notice in the Description page of Project Settings.


#include "SimulatorWorld.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "JsonObjectConverter.h"
#include "Blueprint/UserWidget.h"
#include "MyGameInstance.h"
#include "Math/Vector.h"
#include "Camera.h"
#include "ActorWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
//#include "VWLanePhysicalMaterial.h"
#include "ControlArea.h"
#include "APIService.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "WBP_Notification.h"
#include <Components/BoxComponent.h>

// Sets default values
ASimulatorWorld::ASimulatorWorld()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Http = &FHttpModule::Get();
}

// Called when the game starts or when spawned
void ASimulatorWorld::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("SimWorld BeginPlay"));
	geo = Cast<AGeoReferencingSystem>(UGameplayStatics::GetActorOfClass(GetWorld(), AGeoReferencingSystem::StaticClass()));
	GameInstance = Cast<UMyGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	TopViewCamera = Cast<ACamera>(UGameplayStatics::GetActorOfClass(GetWorld(), ACamera::StaticClass()));

	APIService = Cast<AAPIService>(UGameplayStatics::GetActorOfClass(GetWorld(), AAPIService::StaticClass()));
	Player = Cast<AMyVehicle>(UGameplayStatics::GetActorOfClass(GetWorld(), AMyVehicle::StaticClass()));

	FString ConfigFilePath = FPaths::Combine(*FPaths::ProjectContentDir(), *FString(TEXT("VirtualWorld/Config.ini")));
	GConfig->GetString(TEXT("Config"), TEXT("GoogleSheetAPI"), GoogleSheetAPIURL, ConfigFilePath);
	LoadMap();

	UChaosWheeledVehicleMovementComponent* VehicleMovementComponent = Cast<UChaosWheeledVehicleMovementComponent>(Player->GetVehicleMovementComponent());
	if (VehicleMovementComponent)
	{
		VehicleWidth = VehicleMovementComponent->ChassisWidth;
	}
}

void ASimulatorWorld::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

}

// Called every frame
void ASimulatorWorld::Tick(float DeltaTime)
{

	Super::Tick(DeltaTime);	
	if (IsEnableMoveToGPSLocation)
	{
		RunMovePlayerToGPSLocation(DeltaTime);
	}
	else
	{
		if (PlayerController->GetPawn() != Player)
		{
			PlayerController->Possess(Player);
			UE_LOG(LogTemp, Warning, TEXT("Player controller regained control of the vehicle"));
		}

		IsFirstTimeMove = true;
		FirstMoveList.Empty();
		GPSList.Empty();

		PreviousActorLocation = FVector(0, 0, 0);
		IsFirstTimeGPS = true;
		IsFirstTarget = true;
		ListTargetLocation.Empty();
		ListTargetRotation.Empty();
	}
	if (IsEditorMode && PlacementPreview)
	{
		UpdatePlacementPreviewPosition();
	}

	float DistanceMoved = FVector::Dist(Player->GetActorLocation(), LastPlayerLocation);
	if (DistanceMoved > 1.0f)
	{
		TimeNotMoving = 0.0f;
	}
	else
	{
		TimeNotMoving += DeltaTime;
	}
	LastPlayerLocation = Player->GetActorLocation();
}

//float ASimulatorWorld::FindDistanceToLane(EWheelID wheelID, float direction)
//{
//
//	USkeletalMeshComponent* CarMesh = PlayerTemp->FindComponentByClass<USkeletalMeshComponent>();
//	FVector wheelPosition;
//	float laneDirection = direction;
//
//	if (wheelID == EWheelID::FrontLeft)
//	{
//		wheelPosition = CarMesh->GetBoneLocation(LeftFrontWheelName);
//		//laneDirection = -1.0f;
//	}
//	else
//	{
//		wheelPosition = CarMesh->GetBoneLocation(RightFrontWheelName);
//		//laneDirection = 1.0f;
//	}
//
//	const float Step = 7.5f;
//	const float TestDistance = 8000.0f;
//	const int32 NumSteps = TestDistance / Step;
//	FVector position = wheelPosition;
//	static FName traceTag(TEXT("LaneDetection"));
//	FCollisionQueryParams traceParams(traceTag, true);
//	traceParams.bReturnPhysicalMaterial = true;
//
//	FHitResult hitResult;
//
//#if WITH_EDITOR
//#endif
//
//	const FVector rightMoveDelta = PlayerTemp->GetActorRightVector() * Step * laneDirection;
//
//	for (int32 j = 0; j < NumSteps; ++j)
//	{
//		const FVector startPosition = position + FVector(0.0f, 0.0f, 100.0f);
//		const FVector endPosition = position - FVector(0.0f, 0.0f, 100.0f);
//
//		if (GetWorld()->LineTraceSingleByChannel(hitResult, startPosition, endPosition, LANE_DETECTION_TRACE_CHANNEL, traceParams) != false)
//		{
//			if (UVWLanePhysicalMaterial* laneMat = Cast<UVWLanePhysicalMaterial>(hitResult.PhysMaterial))
//			{
//#if WITH_EDITOR
//				DrawDebugDirectionalArrow(GetWorld(), wheelPosition, hitResult.Location, 16, FColor::Red, false, -1, 0, 2.0f);
//				DrawDebugSphere(GetWorld(), hitResult.Location, 5, 8, FColor::Red, false, -1, 0, 1);
//#endif
//
//				return FVector::Dist2D(wheelPosition, hitResult.Location);
//			}
//		}
//		position += rightMoveDelta;
//	}
//
//	return -1.0f;
//}

void ASimulatorWorld::LoadMap()
{
	FString FullPathDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
	FString MapPathYAML = FPaths::Combine(FullPathDir, TEXT("Content/VirtualWorld/Map.yaml"));

	FMapJson MapDataStruct = {};

	bool status = false;
	FYamlNode Out;
	if (UYamlParsing::LoadYamlFromFile(MapPathYAML, Out))
	{
		status = ParseNodeIntoStruct(Out, MapDataStruct);

		FActorSpawnParameters SpawnInfo;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		int BuildingNum = MapDataStruct.mapDatalist.congestionDatalist.Num();
		for (int BuildingIdx = 0; BuildingIdx < BuildingNum; BuildingIdx++)
		{
			FString BluePrintPath = ("/Game/VirtualWorld/Actor/BP_ControlArea.BP_ControlArea_C");
			UClass* BluePrintClass = StaticLoadClass(UObject::StaticClass(), NULL, *BluePrintPath);

			FVector  Location = FVector::ZeroVector;
			FRotator Rotation = FRotator::ZeroRotator;
			FVector Scale = FVector::ZeroVector;

			Location.X = MapDataStruct.mapDatalist.congestionDatalist[BuildingIdx].positionX;
			Location.Y = MapDataStruct.mapDatalist.congestionDatalist[BuildingIdx].positionY;
			Location.Z = MapDataStruct.mapDatalist.congestionDatalist[BuildingIdx].positionZ;

			Rotation.Roll = MapDataStruct.mapDatalist.congestionDatalist[BuildingIdx].rotationX;
			Rotation.Yaw = MapDataStruct.mapDatalist.congestionDatalist[BuildingIdx].rotationY;
			Rotation.Pitch = MapDataStruct.mapDatalist.congestionDatalist[BuildingIdx].rotationZ;

			Scale.X = MapDataStruct.mapDatalist.congestionDatalist[BuildingIdx].scaleX;
			Scale.Y = MapDataStruct.mapDatalist.congestionDatalist[BuildingIdx].scaleZ;
			Scale.Z = MapDataStruct.mapDatalist.congestionDatalist[BuildingIdx].scaleY;

			AActor* spawnActor = GetWorld()->SpawnActor(BluePrintClass, &Location, &Rotation, SpawnInfo);
			spawnActor->SetActorScale3D(Scale);

			FString Name = MapDataStruct.mapDatalist.congestionDatalist[BuildingIdx].congestionName;
			int Fee = MapDataStruct.mapDatalist.congestionDatalist[BuildingIdx].congestionFee;
			AControlArea* CongestionActor = Cast<AControlArea>(spawnActor);
			CongestionActor->Name = Name;
			CongestionActor->Fee = Fee;

		}
	}
}


void ASimulatorWorld::DisablePhysic()
{

	Player->GetMesh()->SetSimulatePhysics(false);
}


void ASimulatorWorld::EnablePhysic()
{
	Player->GetMesh()->RecreatePhysicsState();
	Player->GetMesh()->SetSimulatePhysics(true);
}

void ASimulatorWorld::SaveMap()
{
	FMapJson mapDataStruct;
	TArray<FCongestionSructure> CongestionList;

	TArray<AActor*> FoundBuilding;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AControlArea::StaticClass(), FoundBuilding);
	int FoundBuildingNum = FoundBuilding.Num();
	for (int FoundBuildingIdx = 0; FoundBuildingIdx < FoundBuildingNum; FoundBuildingIdx++)
	{
		FCongestionSructure ThisBuilding;
		AControlArea* ControlAreaBuilding = Cast<AControlArea>(FoundBuilding[FoundBuildingIdx]);
		ThisBuilding.congestionName = ControlAreaBuilding->Name;
		ThisBuilding.congestionFee = ControlAreaBuilding->Fee;
		ThisBuilding.positionX = FoundBuilding[FoundBuildingIdx]->GetActorLocation().X;
		ThisBuilding.positionY = FoundBuilding[FoundBuildingIdx]->GetActorLocation().Y;
		ThisBuilding.positionZ = FoundBuilding[FoundBuildingIdx]->GetActorLocation().Z;
		ThisBuilding.rotationX = FoundBuilding[FoundBuildingIdx]->GetActorRotation().Roll;
		ThisBuilding.rotationY = FoundBuilding[FoundBuildingIdx]->GetActorRotation().Yaw;
		ThisBuilding.rotationZ = FoundBuilding[FoundBuildingIdx]->GetActorRotation().Pitch;
		ThisBuilding.scaleX = FoundBuilding[FoundBuildingIdx]->GetActorScale().X;
		ThisBuilding.scaleY = FoundBuilding[FoundBuildingIdx]->GetActorScale().Z;
		ThisBuilding.scaleZ = FoundBuilding[FoundBuildingIdx]->GetActorScale().Y;
		CongestionList.Add(ThisBuilding);
	}

	mapDataStruct.mapDatalist.congestionDatalist = CongestionList;

	FString fullPathDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
	FString mapPathYAML = FPaths::Combine(fullPathDir, TEXT("Content/VirtualWorld/Map.yaml"));

	FYamlNode Out;

	if (ParseStructToNode(Out, mapDataStruct))
	{
		UYamlParsing::WriteYamlToFile(mapPathYAML, Out);
	}
}

//FVector ASimulatorWorld::GetCTargetLocation()
//{
//
//	float leftDistanceToRoad = 0;
//	float rightDistanceToRoad = 0;
//
//	FVector beforeCurrentLocation = PlayerTemp->GetActorLocation();
//	FRotator beforeCurrentDirection = PlayerTemp->GetActorRotation();
//
//	FVector VectorRotation = (PreviousActorLocation - PlayerTemp->GetActorLocation());
//	VectorRotation.Normalize();
//	PlayerTemp->SetActorLocation(PreviousActorLocation, false, nullptr, ETeleportType::TeleportPhysics);
//	PlayerTemp->SetActorRotation(VectorRotation.Rotation(), ETeleportType::TeleportPhysics);
//	IsFirstTimeGPS = false;
//
//	leftDistanceToRoad = FindDistanceToLane(EWheelID::FrontLeft, -1.0f);
//	rightDistanceToRoad = FindDistanceToLane(EWheelID::FrontRight, 1.0f);
//	FVector currentLocation = PlayerTemp->GetActorLocation();
//	FVector currentDirection = PlayerTemp->GetActorForwardVector();
//
//	FString direction = "north";
//	if (currentDirection.X >= 0)
//	{
//		if (currentDirection.Y >= -0.5 && currentDirection.Y < 0.5)
//		{
//			direction = "east";
//		}
//		else if (currentDirection.Y < -0.5)
//		{
//			direction = "north";
//		}
//		else // currentDirection.Y > 0.5
//		{
//			direction = "south";
//		}
//	}
//	else
//	{
//		if (currentDirection.Y >= -0.5 && currentDirection.Y < 0.5)
//		{
//			direction = "west";
//		}
//		else if (currentDirection.Y < -0.5)
//		{
//			direction = "north";
//		}
//		else // currentDirection.Y > 0.5
//		{
//			direction = "south";
//		}
//	}
//
//	if (leftDistanceToRoad < 0 && rightDistanceToRoad == 0)
//	{
//		leftDistanceToRoad = FindDistanceToLane(EWheelID::FrontLeft, 1.0f);
//		if (direction == "north")
//		{
//			PlayerTemp->SetActorLocation(currentLocation + FVector(leftDistanceToRoad + 50, 0, 0), false, nullptr, ETeleportType::TeleportPhysics);
//		}
//		else if (direction == "south")
//		{
//			PlayerTemp->SetActorLocation(currentLocation - FVector(leftDistanceToRoad + 50, 0, 0), false, nullptr, ETeleportType::TeleportPhysics);
//		}
//		else if (direction == "east")
//		{
//			PlayerTemp->SetActorLocation(currentLocation + FVector(0, leftDistanceToRoad + 50, 0), false, nullptr, ETeleportType::TeleportPhysics);
//		}
//		else // west
//		{
//			PlayerTemp->SetActorLocation(currentLocation - FVector(0, leftDistanceToRoad + 50, 0), false, nullptr, ETeleportType::TeleportPhysics);
//		}
//
//	}
//	else if (leftDistanceToRoad == 0 && rightDistanceToRoad < 0)
//	{
//		rightDistanceToRoad = FindDistanceToLane(EWheelID::FrontRight, -1.0f);
//
//		if (direction == "north")
//		{
//			PlayerTemp->SetActorLocation(currentLocation - FVector(rightDistanceToRoad + 50, 0, 0), false, nullptr, ETeleportType::TeleportPhysics);
//		}
//		else if (direction == "south")
//		{
//			PlayerTemp->SetActorLocation(currentLocation + FVector(rightDistanceToRoad + 50, 0, 0), false, nullptr, ETeleportType::TeleportPhysics);
//		}
//		else if (direction == "east")
//		{
//			PlayerTemp->SetActorLocation(currentLocation - FVector(0, rightDistanceToRoad + 50, 0), false, nullptr, ETeleportType::TeleportPhysics);
//		}
//		else // west
//		{
//			PlayerTemp->SetActorLocation(currentLocation + FVector(0, rightDistanceToRoad + 50, 0), false, nullptr, ETeleportType::TeleportPhysics);
//		}
//	}
//	else if ((leftDistanceToRoad < 0 && rightDistanceToRoad > 0) || leftDistanceToRoad > 0 && rightDistanceToRoad > 0 && leftDistanceToRoad > rightDistanceToRoad)
//	{
//		if (direction == "north")
//		{
//			PlayerTemp->SetActorLocation(currentLocation + FVector(rightDistanceToRoad + 250, 0, 0), false, nullptr, ETeleportType::TeleportPhysics);
//		}
//		else if (direction == "south")
//		{
//			PlayerTemp->SetActorLocation(currentLocation - FVector(rightDistanceToRoad + 250, 0, 0), false, nullptr, ETeleportType::TeleportPhysics);
//		}
//		else if (direction == "east")
//		{
//			PlayerTemp->SetActorLocation(currentLocation + FVector(0, rightDistanceToRoad + 250, 0), false, nullptr, ETeleportType::TeleportPhysics);
//		}
//		else // west
//		{
//			PlayerTemp->SetActorLocation(currentLocation - FVector(0, rightDistanceToRoad + 250, 0), false, nullptr, ETeleportType::TeleportPhysics);
//		}
//	}
//	else if ((leftDistanceToRoad > 0 && rightDistanceToRoad < 0) || leftDistanceToRoad > 0 && rightDistanceToRoad > 0 && leftDistanceToRoad <= rightDistanceToRoad)
//	{
//		if (direction == "north")
//		{
//			PlayerTemp->SetActorLocation(currentLocation - FVector(leftDistanceToRoad + 250, 0, 0), false, nullptr, ETeleportType::TeleportPhysics);
//		}
//		else if (direction == "south")
//		{
//			PlayerTemp->SetActorLocation(currentLocation + FVector(leftDistanceToRoad + 250, 0, 0), false, nullptr, ETeleportType::TeleportPhysics);
//		}
//		else if (direction == "east")
//		{
//			PlayerTemp->SetActorLocation(currentLocation - FVector(0, leftDistanceToRoad + 250, 0), false, nullptr, ETeleportType::TeleportPhysics);
//		}
//		else // west
//		{
//			PlayerTemp->SetActorLocation(currentLocation + FVector(0, leftDistanceToRoad + 250, 0), false, nullptr, ETeleportType::TeleportPhysics);
//		}
//	}
//
//
//	TempTargetLocation = PlayerTemp->GetActorLocation();
//	TempTargetRotation = PlayerTemp->GetActorRotation();
//
//	return TempTargetLocation;
//}
//


//void ASimulatorWorld::MoveLocation(FVector playerLocation)
//{
//
//	float leftDistanceToRoad = 0;
//	float rightDistanceToRoad = 0;
//
//	FVector beforeCurrentLocation = Player->GetActorLocation();
//	FRotator beforeCurrentDirection = Player->GetActorRotation();
//	FVector targetLocation;
//
//	FVector VectorRotation = (PreviousActorLocation - Player->GetActorLocation());
//	VectorRotation.Normalize();
//	Player->SetActorLocation(PreviousActorLocation, false, nullptr, ETeleportType::TeleportPhysics);
//	Player->SetActorRotation(VectorRotation.Rotation(), ETeleportType::TeleportPhysics);
//	IsFirstTimeGPS = false;
//
//	leftDistanceToRoad = FindDistanceToLane(EWheelID::FrontLeft, -1.0f);
//	rightDistanceToRoad = FindDistanceToLane(EWheelID::FrontRight, 1.0f);
//	FVector currentLocation = Player->GetActorLocation();
//	FVector currentDirection = Player->GetActorForwardVector();
//
//	FString direction = "north";
//
//	if (currentDirection.X >= 0)
//	{
//		if (currentDirection.Y >= -0.5 && currentDirection.Y < 0.5)
//		{
//			direction = "east";
//		}
//		else if (currentDirection.Y < -0.5)
//		{
//			direction = "north";
//		}
//		else // currentDirection.Y > 0.5
//		{
//			direction = "south";
//		}
//	}
//	else
//	{
//		if (currentDirection.Y >= -0.5 && currentDirection.Y < 0.5)
//		{
//			direction = "west";
//		}
//		else if (currentDirection.Y < -0.5)
//		{
//			direction = "north";
//		}
//		else // currentDirection.Y > 0.5
//		{
//			direction = "south";
//		}
//	}
//
//	if (leftDistanceToRoad < 0 && rightDistanceToRoad == 0)
//	{
//		leftDistanceToRoad = FindDistanceToLane(EWheelID::FrontLeft, 1.0f);
//		if (direction == "north")
//		{
//			Player->SetActorLocation(currentLocation + FVector(leftDistanceToRoad + 50, 0, 0), false, nullptr, ETeleportType::TeleportPhysics);
//		}
//		else if (direction == "south")
//		{
//			Player->SetActorLocation(currentLocation - FVector(leftDistanceToRoad + 50, 0, 0), false, nullptr, ETeleportType::TeleportPhysics);
//		}
//		else if (direction == "east")
//		{
//			Player->SetActorLocation(currentLocation + FVector(0, leftDistanceToRoad + 50, 0), false, nullptr, ETeleportType::TeleportPhysics);
//		}
//		else // west
//		{
//			Player->SetActorLocation(currentLocation - FVector(0, leftDistanceToRoad + 50, 0), false, nullptr, ETeleportType::TeleportPhysics);
//		}
//
//	}
//	else if (leftDistanceToRoad == 0 && rightDistanceToRoad < 0)
//	{
//		rightDistanceToRoad = FindDistanceToLane(EWheelID::FrontRight, -1.0f);
//
//		if (direction == "north")
//		{
//			Player->SetActorLocation(currentLocation - FVector(rightDistanceToRoad + 50, 0, 0), false, nullptr, ETeleportType::TeleportPhysics);
//		}
//		else if (direction == "south")
//		{
//			Player->SetActorLocation(currentLocation + FVector(rightDistanceToRoad + 50, 0, 0), false, nullptr, ETeleportType::TeleportPhysics);
//		}
//		else if (direction == "east")
//		{
//			Player->SetActorLocation(currentLocation - FVector(0, rightDistanceToRoad + 50, 0), false, nullptr, ETeleportType::TeleportPhysics);
//		}
//		else // west
//		{
//			Player->SetActorLocation(currentLocation + FVector(0, rightDistanceToRoad + 50, 0), false, nullptr, ETeleportType::TeleportPhysics);
//		}
//	}
//	else if ((leftDistanceToRoad < 0 && rightDistanceToRoad > 0) || leftDistanceToRoad > 0 && rightDistanceToRoad > 0 && leftDistanceToRoad > rightDistanceToRoad)
//	{
//		if (direction == "north")
//		{
//			Player->SetActorLocation(currentLocation + FVector(rightDistanceToRoad + 250, 0, 0), false, nullptr, ETeleportType::TeleportPhysics);
//		}
//		else if (direction == "south")
//		{
//			Player->SetActorLocation(currentLocation - FVector(rightDistanceToRoad + 250, 0, 0), false, nullptr, ETeleportType::TeleportPhysics);
//		}
//		else if (direction == "east")
//		{
//			Player->SetActorLocation(currentLocation + FVector(0, rightDistanceToRoad + 250, 0), false, nullptr, ETeleportType::TeleportPhysics);
//		}
//		else // west
//		{
//			Player->SetActorLocation(currentLocation - FVector(0, rightDistanceToRoad + 250, 0), false, nullptr, ETeleportType::TeleportPhysics);
//		}
//	}
//	else if ((leftDistanceToRoad > 0 && rightDistanceToRoad < 0) || leftDistanceToRoad > 0 && rightDistanceToRoad > 0 && leftDistanceToRoad <= rightDistanceToRoad)
//	{
//		if (direction == "north")
//		{
//			Player->SetActorLocation(currentLocation - FVector(leftDistanceToRoad + 250, 0, 0), false, nullptr, ETeleportType::TeleportPhysics);
//		}
//		else if (direction == "south")
//		{
//			Player->SetActorLocation(currentLocation + FVector(leftDistanceToRoad + 250, 0, 0), false, nullptr, ETeleportType::TeleportPhysics);
//		}
//		else if (direction == "east")
//		{
//			Player->SetActorLocation(currentLocation - FVector(0, leftDistanceToRoad + 250, 0), false, nullptr, ETeleportType::TeleportPhysics);
//		}
//		else // west
//		{
//			Player->SetActorLocation(currentLocation + FVector(0, leftDistanceToRoad + 250, 0), false, nullptr, ETeleportType::TeleportPhysics);
//		}
//	}
//
//
//	targetLocation = Player->GetActorLocation();
//
//}

void ASimulatorWorld::MovePlayerToGPSLocation(FString latitude, FString longtitude, float DeltaTime)
{
	//Convert lat long to x,y,z in unreal.
	FGeographicCoordinates geoStruct;
	FVector worldLocation;
	FVector playerLocation;
	geoStruct.Altitude = TempAltitude;
	geoStruct.Latitude = FCString::Atod(*latitude);
	geoStruct.Longitude = FCString::Atod(*longtitude);
	geo->GeographicToECEF(geoStruct, worldLocation);
	geo->ECEFToEngine(worldLocation, playerLocation);

	//Prevent the car from sinking into the ground.
	playerLocation.Z += 100;
	
	//Can see raw destination with uncomment below code.
	//DrawDebugSphere(GetWorld(), playerLocation, 100.0f, 5, FColor::Red, false, 10.0f);


	//Change raw destination to destination on NavMesh.
	FVector NewplayerLocation = GetCloseLocationOnNavMesh(playerLocation);

	//Set vehicle for start move
	if (IsFirstTimeMove == true) {
		CreateFirstMoveList(NewplayerLocation);
	}

	DistanceToLastTarget = FVector::Dist(Player->GetActorLocation(), NewplayerLocation);

	if (IsFirstTimeMove == true && FirstMoveList.Num() == 4.0f) {
		FVector Direction = (FirstMoveList[2] - FirstMoveList[1]).GetSafeNormal();
		FRotator Rotation = Direction.Rotation();
		FirstMoveList[1].Z = 200;
		Player->SetActorLocation(FirstMoveList[1], false, nullptr, ETeleportType::TeleportPhysics);
		Player->SetActorRotation(Rotation, ETeleportType::TeleportPhysics);
		IsFirstTimeMove = false;
		TimeNotMoving = 0.0f;
		GPSList.Add(FirstMoveList[1]);
		GPSList.Add(FirstMoveList[2]);
		GPSList.Add(FirstMoveList[3]);
		FirstMoveList.Empty();
	}

	//Reset AIMove if vehicle doesn't move 20 sec and distance to last target more than 100 m.
	else if (IsFirstTimeMove == false && DistanceToLastTarget >= 10000 && TimeNotMoving > 20.0f) {
		ResetAIMove();
	}

	//If next destination more than 5 m and doesn't behind vehicle add that destination to GPSList for move.
	//The next destination must be more than 5m because of GPS redundancy.
	else if (IsFirstTimeMove == false && DistanceToLastTarget >= 500 && HandleGPSBehindVehicle(NewplayerLocation)) {
		CreateGPSList(NewplayerLocation);
		if (GPSList.Num() >= BufferLength) {	
			if (IsWrongRoad() && DestinationIsOnLeftSideOfVehicle(GPSList[0])) {
				ResetAIMove();
			}
			else if (IsWrongRoad()) {
				GPSList[0] = HandleGPSWrongRoad(GPSList[0]);
				//DrawDebugSphere(GetWorld(), GPSList[0], 100.0f, 5, FColor::Blue, false, 10.0f);
				Player->HandleMovement(GPSList[0]);
				GPSList.RemoveAt(0);
			}
			else {
				//DrawDebugSphere(GetWorld(), GPSList[0], 100.0f, 5, FColor::Blue, false, 10.0f);
				Player->HandleMovement(GPSList[0]);
				GPSList.RemoveAt(0);
			}
		}
	}

	//Reset Vehicle if it don't move more than 5 sec and have next destination.
	if (IsFirstTimeMove == false && TimeNotMoving > 5.0f && GPSList.Num() > 0) {
		if (IsWrongRoad()) {
			GPSList.Last() = HandleGPSWrongRoad(GPSList.Last());
		}
		Player->HandleMovement(GPSList.Last());
		GPSList.Empty();
	}

	//Blend camera.
	if (PlayerController->GetViewTarget() != Player)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerController->GetViewTarget() != Player"));
		// Set the new view target with blend
		PlayerController->SetViewTargetWithBlend(Player);
	}
	IsStartGetGPS = false;
	IsCompletedGetGPS = false;
	Timer = 0;
}

void ASimulatorWorld::OnGetTimeResponse(FHttpRequestPtr req, FHttpResponsePtr res, bool isConnectSuccessfull)
{
	if (isConnectSuccessfull)
	{
		TSharedPtr<FJsonObject> JsonObject;
		// Check if connect to server
		if (res->GetResponseCode() == 200)
		{
			const FString ResponseBody = res->GetContentAsString();
			UE_LOG(LogTemp, Log, TEXT("Received GPS Data: %s"), *ResponseBody);
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);
			// Chack if read json successfully
			if (FJsonSerializer::Deserialize(Reader, JsonObject))
			{
				CurrentTargetLatitude = JsonObject->GetStringField("lat");
				CurrentTargetLongtitude = JsonObject->GetStringField("long");
				IsCompletedGetGPS = true;

			}
		}
	}
	
}

void ASimulatorWorld::OnPostTimeResponse(FHttpRequestPtr req, FHttpResponsePtr res, bool isConnectSuccessfull)
{
	IsStartPostControlAreaStatus = false;
}

void ASimulatorWorld::GetGPSFromServer()
{
	IsStartGetGPS = true;

	if (APIService)
	{
		TSharedPtr<FJsonObject> JsonObject;
		FString GPSData = APIService->GetGPSData();
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(GPSData);
		// Chack if read json successfully
		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			CurrentTargetLatitude = JsonObject->GetStringField("lat");
			CurrentTargetLongtitude = JsonObject->GetStringField("long");
			IsCompletedGetGPS = true;
		}
		else{
			IsStartGetGPS = false;
			UE_LOG(LogTemp, Log, TEXT("Unable to Fetch data yet"));
		}
	}
}

void ASimulatorWorld::PostControlAreaStatusToServer(bool InArea, int Fee, FString name)
{
	IsStartPostControlAreaStatus = true;
	if (APIService)
	{
		APIService->PostNotification(InArea, Fee, name);
		IsStartPostControlAreaStatus = false;
	}
}

void ASimulatorWorld::RunMovePlayerToGPSLocation(float DeltaTime)
{
	if (IsStartGetGPS == false) {
		GetGPSFromServer();
	}
	if (IsCompletedGetGPS == true) {
		MovePlayerToGPSLocation(CurrentTargetLatitude, CurrentTargetLongtitude, DeltaTime);
	}
}

void ASimulatorWorld::StopMovePlayerToGPSLocation()
{
	IsEnableMoveToGPSLocation = false;
	IsCompletedGetGPS = false;
	Timer = 0;
}

void ASimulatorWorld::HideAllWidgetFromInput(UUserWidget* widgetInput)
{
	GameInstance = Cast<UMyGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	for (UUserWidget* temp : GameInstance->AllWidget) {
		if (widgetInput == temp)
		{
			temp->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			temp->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ASimulatorWorld::UnHideAllWidget()
{
	for (UUserWidget* temp : GameInstance->AllWidget) {
		temp->SetVisibility(ESlateVisibility::Visible);
		
	}
	FInputModeGameAndUI inputMode;
	PlayerController->SetInputMode(inputMode);
}

void ASimulatorWorld::EnableTopViewCameraMode()
{
	PlayerController->SetViewTargetWithBlend(TopViewCamera);
}

void ASimulatorWorld::DisableTopViewCameraMode()
{
	PlayerController->SetViewTargetWithBlend(Player);
}

void ASimulatorWorld::UpdatePlacementPreviewPosition()
{
	if (PlacementPreview == nullptr)
		return;

	FVector cameraPos, cameraDirection;
	if (PlayerController->DeprojectMousePositionToWorld(cameraPos, cameraDirection) == false)
		return;
	static FName traceTag(TEXT("UpdatePlacement"));
	FCollisionQueryParams traceParams(traceTag, true, PlacementPreview);

	FHitResult hitResult;
	bool HitFlag = false;

	HitFlag = GetWorld()->LineTraceSingleByChannel(hitResult, cameraPos + cameraDirection * 10.0f, cameraPos + cameraDirection * 1000000000000000.0f, ECC_Visibility, traceParams);

	if (!HitFlag)
		return;

	// Fix value to set on landscape
	hitResult.Location.Z = 100;
	PlacementPreview->SetActorLocation(hitResult.Location);
}

AActor* ASimulatorWorld::SpawnNewObjectAtPlace()
{
	if (PlacementPreview == nullptr)
		return nullptr;

	FVector position;
	FRotator rotation;
	FVector  scale;

	if (!IsCancleEditing)
	{
		position = PlacementPreview->GetActorLocation();
		rotation = PlacementPreview->GetActorRotation();
		scale = PlacementPreview->GetActorScale();
	}
	else
	{
		position = BeforeEditingPosition;
		rotation = BeforeEditingRotation;
		scale = BeforeEditingScale;
	}

	FString PlacementPreviewClassName = PlacementPreview->GetClass()->GetName();
	if(PlacementPreviewClassName == "BP_ControlArea_C")
	{
		FActorSpawnParameters spawnInfo;
		spawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActor* object = GetWorld()->SpawnActor(PlacementPreview->GetClass(), &position, &rotation, spawnInfo);
		if (object != nullptr) object->SetActorScale3D(scale);

		return object;
	}
	else
	{
		return nullptr;
	}
	
}

void ASimulatorWorld::YawRotatePlacementPreview(float Value)
{
	if (PlacementPreview && PlacementPreview->GetClass()->GetName() == "BP_ControlArea_C")
	{
		FRotator HeadRotation;
		HeadRotation = PlacementPreview->GetActorRotation();
		if (Value != 0)
		{
			HeadRotation.Yaw += Value * 15 * GetWorld()->GetDeltaSeconds();
		}
		PlacementPreview->SetActorRotation(HeadRotation);
	}
	
}

void ASimulatorWorld::PitchRotatePlacementPreview(float Value)
{
	if (PlacementPreview && PlacementPreview->GetClass()->GetName() == "BP_ControlArea_C")
	{
		FRotator HeadRotation;
		HeadRotation = PlacementPreview->GetActorRotation();
		if (Value != 0)
		{
			HeadRotation.Pitch += Value * 15 * GetWorld()->GetDeltaSeconds();
		}
		PlacementPreview->SetActorRotation(HeadRotation);
	}
}

void ASimulatorWorld::ScaleWidthPlacementPreview(float Value)
{
	if (PlacementPreview && PlacementPreview->GetClass()->GetName() == "BP_ControlArea_C")
	{
		FVector scale = PlacementPreview->GetActorScale();

		scale.X += GetWorld()->GetDeltaSeconds() * Value * 1.0f;
		if (scale.X < 0.01f)
			scale.X = 0.01f;

		PlacementPreview->SetActorScale3D(scale);
	}
	
}

void ASimulatorWorld::ScaleLengthPlacementPreview(float Value)
{
	if (PlacementPreview && PlacementPreview->GetClass()->GetName() == "BP_ControlArea_C")
	{
		FVector scale = PlacementPreview->GetActorScale();

		scale.Y += GetWorld()->GetDeltaSeconds() * Value * 1.0f;
		if (scale.Y < 0.01f)
			scale.Y = 0.01f;

		PlacementPreview->SetActorScale3D(scale);
	}
}


bool ASimulatorWorld::CheckPlacementOverlap(AActor* previewActor)
{
	bool OverlapStatus = false;
	TArray<AActor*> OverlapActors;
	PlacementPreview->GetOverlappingActors(OverlapActors);

	for (int idx = 0; idx < OverlapActors.Num(); idx++)
	{
		if (OverlapActors[idx]->GetClass()->GetName().StartsWith("BP_ControlArea"))
		{
			OverlapStatus = true;
			break;
		}
	}

	return OverlapStatus;
}

bool ASimulatorWorld::CheckPlacementOverlapRoad(AActor* previewActor)
{
	bool OverlapStatus = false;
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

	UBoxComponent* BoxComp = previewActor->FindComponentByClass<UBoxComponent>();
	UPrimitiveComponent* TargetComponent = BoxComp;

	// Get the component's bounding box and location
	FBox ComponentBoundingBox = TargetComponent->Bounds.GetBox();
	FVector ComponentLocation = TargetComponent->GetComponentLocation();
	FVector ComponentExtent = ComponentBoundingBox.GetExtent();
	FNavLocation ProjectedLocation;

	if (NavSys->ProjectPointToNavigation(ComponentLocation, ProjectedLocation, ComponentExtent)) {
		OverlapStatus = true;
	}
	return OverlapStatus;
}

void ASimulatorWorld::OnMouseLeftClick()
{
	if (IsEditorMode && PlacementPreview)
	{
		if (CheckPlacementOverlap(PlacementPreview))
		{
			// skip placement

			UWBP_Notification* NotificationWidget = CreateWidget<UWBP_Notification>(GetWorld(), NotificationWidgetClass);
			NotificationWidget->AddToViewport();
			FText NewText = FText::FromString("Congrestion zone overlap other zone can't place ");
			NotificationWidget->ShowNotification(NewText);
		}
		else if (!CheckPlacementOverlapRoad(PlacementPreview)) 
		{
			UWBP_Notification* NotificationWidget = CreateWidget<UWBP_Notification>(GetWorld(), NotificationWidgetClass);
			NotificationWidget->AddToViewport();
			FText NewText = FText::FromString("Congrestion zone don't overlap road can't place ");
			NotificationWidget->ShowNotification(NewText);
		}
		else
		{
			SpawnNewObjectAtPlace();
		}
	}
}

void ASimulatorWorld::OnESCKey()
{
	if (IsEditorMode && PlacementPreview)
	{
		TArray<UUserWidget*> WidgetList;
		FSoftClassPath EditorClassRef(FString(TEXT("/Game/VirtualWorld/Widget/WBP_Editor.WBP_Editor_C")));
		TSubclassOf<UUserWidget> Widget = EditorClassRef.TryLoadClass<UUserWidget>();
		UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), WidgetList, Widget);
		UUserWidget* EditorWidget = nullptr;
		if (WidgetList.Num() > 0)
			EditorWidget = WidgetList[0];


		if (EditorWidget != nullptr)
		{
			UButton* ControlAreaButton = Cast<UButton>(EditorWidget->WidgetTree->FindWidget("Area_Button"));
			ControlAreaButton->OnClicked.Broadcast();
		}
	}
}

void ASimulatorWorld::StopControlAreaPlacement() {

		IsSpawndEditedActor = true;
		PlacementPreview->Destroy();
		PlacementPreview = nullptr;

}

void ASimulatorWorld::OnClickControlArea(AActor* ControlArea)
{
	FSoftClassPath TrafficLightClassRef(PATH_WBP_ControlAreaSetting_C);
	TSubclassOf<UUserWidget> Widget = TrafficLightClassRef.TryLoadClass<UUserWidget>();
	if (IsValid(Widget))
	{
		UUserWidget* ControlAreaSettingWidget = Cast<UUserWidget>(CreateWidget(GetWorld(), Widget));
		if (ControlAreaSettingWidget != nullptr)
		{
			UActorWidget* ActorWidget = Cast<UActorWidget>(ControlAreaSettingWidget);
			if (ActorWidget != nullptr)
			{
				ActorWidget->OwningActor = ControlArea;
			}
			AControlArea* CControlArea = Cast<AControlArea>(ActorWidget->OwningActor);
			UEditableTextBox* FeeText = Cast<UEditableTextBox>(ControlAreaSettingWidget->WidgetTree->FindWidget(FName("FeeTextInput")));
			FeeText->SetText(FText::FromString(FString::FromInt(CControlArea->Fee)));
			UEditableTextBox* NameText = Cast<UEditableTextBox>(ControlAreaSettingWidget->WidgetTree->FindWidget(FName("NameTextInput")));
			NameText->SetText(FText::FromString(CControlArea->Name));
			FVector2D WidgetLocation;
			PlayerController->ProjectWorldLocationToScreen(CControlArea->GetActorLocation(), WidgetLocation, true);
			ControlAreaSettingWidget->SetPositionInViewport(WidgetLocation, true);
			ControlAreaSettingWidget->AddToViewport();
			ControlAreaSettingWidget->SetDesiredSizeInViewport(WidgetLocation);
			ControlAreaSettingWidget->SetPositionInViewport(WidgetLocation, true);
			
		}
	}
}


void ASimulatorWorld::SpawnPlacementPreviewActor(TSubclassOf<AActor> ObjectClass)
{
	IsSpawndEditedActor = false;
	if (PlacementPreview != nullptr)
	{
		PlacementPreview->Destroy();
		PlacementPreview = nullptr;
	}

	FActorSpawnParameters spawnInfo;
	spawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (!IsPlacedActorEditing) {
		PlacementPreview = GetWorld()->SpawnActor(ObjectClass, &FVector::ZeroVector, &FRotator::ZeroRotator, spawnInfo);
	}
	else
	{
		PlacementPreview = GetWorld()->SpawnActor(ObjectClass, &FVector::ZeroVector, &BeforeEditingRotation, spawnInfo);
		if (PlacementPreview != nullptr) 
			PlacementPreview->SetActorScale3D(BeforeEditingScale);
	}

	if (PlacementPreview != nullptr) {
		PlacementPreview->DisableComponentsSimulatePhysics();
	}
}


void ASimulatorWorld::CreateFirstMoveList(FVector& NewplayerLocation) {
	bool bLocationOnNavmesh = CheckLocationOnNavMesh(NewplayerLocation);

	if (FirstMoveList.Num() == 0) {
		FirstMoveList.Add(NewplayerLocation);
	}
	else if (!(FirstMoveList.Contains(NewplayerLocation)) &&
		bLocationOnNavmesh == true &&
		FVector::Dist(FirstMoveList.Last(), NewplayerLocation) > 200.0f ) // 2 m
	{
		FirstMoveList.Add(NewplayerLocation);
	 }
}

void ASimulatorWorld::CreateGPSList(FVector& NewplayerLocation) {

	//GPS list is created for future development. So now it's only 1 size.
	//Can Chang GPSlist size by change BufferLength in SimulateWorld.h 
	bool bLocationOnNavmesh = CheckLocationOnNavMesh(NewplayerLocation);

	if (GPSList.Num() == 0) {
		GPSList.Add(NewplayerLocation);
	}
	else if (FVector::Dist(GPSList.Last(), NewplayerLocation) > 100.0f && bLocationOnNavmesh == true )
	{
		GPSList.Add(NewplayerLocation);
	}
}

FVector ASimulatorWorld::GetCloseLocationOnNavMesh(FVector& Destination)
{
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSys)
	{
		FNavLocation ProjectedLocation;
		FVector Extent(2000.0f, 2000.0f, 2000.0f);
		if (NavSys->ProjectPointToNavigation(Destination, ProjectedLocation, Extent))
		{
			// Calculate direction from PlayerLocation to Navmesh
			FVector Direction = (ProjectedLocation.Location - Destination).GetSafeNormal();

			// Move to center NavMesh VehicleWidth/2
			FVector NewLocation = ProjectedLocation.Location + (Direction * VehicleWidth/2);
			return NewLocation;
		}
	}
	return Destination;
}

bool ASimulatorWorld::CheckLocationOnNavMesh(FVector& Destination)
{
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSys)
	{
		FNavLocation ProjectedLocation;
		FVector Extent(2000.0f, 2000.0f, 1000.0f);
		if (NavSys->ProjectPointToNavigation(Destination, ProjectedLocation, Extent))
		{
			return true;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("There is a problem with GPS location."));
	return false;
}

float ASimulatorWorld::GetPathLengthToDestination(const FVector& Destination, const FVector* DefaultStartLocation /*= nullptr*/)
{
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSys)
	{
		FVector StartLocation = DefaultStartLocation ? *DefaultStartLocation : Player->GetActorLocation();

		UNavigationPath* NavPath = NavSys->FindPathToLocationSynchronously(GetWorld(), StartLocation, Destination);
		if (NavPath && NavPath->IsValid())
		{
			float TotalPathLength = 0.0f;

			for (int32 PointIndex = 0; PointIndex < NavPath->PathPoints.Num() - 1; ++PointIndex)
			{
				FVector PointA = NavPath->PathPoints[PointIndex];
				FVector PointB = NavPath->PathPoints[PointIndex + 1];

				float SegmentLength = FVector::Dist(PointA, PointB);

				TotalPathLength += SegmentLength;
			}

			return TotalPathLength;
		}
	}

	return 0.0f;
}



bool ASimulatorWorld::HandleGPSBehindVehicle(FVector& Destination)
{
	//This function detects GPS errors that send the location to the behind of the vehicle.

	FVector PlayerForwardVector = Player->GetActorForwardVector();
	FVector DirectionToNewLocation = (Destination - Player->GetActorLocation()).GetSafeNormal();
	float DotProduct = FVector::DotProduct(PlayerForwardVector, DirectionToNewLocation);

	if (DotProduct > 0) {
		return true;
	}
	else {
		return false;
	}
}


FVector ASimulatorWorld::HandleGPSWrongRoad(FVector& Destination, const FVector* Vector /*= nullptr*/)
{
	//This function moves the destination to the front of the vehicle.

	FVector VehicleLocation = Player->GetActorLocation();
	FVector ForwardVector = Vector ? *Vector : Player->GetActorForwardVector();
	FVector DirectionToNewLocation = Destination - VehicleLocation;
	float DistanceInFront = FVector::DotProduct(DirectionToNewLocation, ForwardVector);
	FVector RedirectedLocation = VehicleLocation + (ForwardVector * DistanceInFront);

	Destination = RedirectedLocation;
	Destination = GetCloseLocationOnNavMesh(Destination);

	//DrawDebugSphere(GetWorld(), Destination, 100.0f, 5, FColor::Green, false, 10.0f);

	return Destination;
}

bool ASimulatorWorld::IsWrongRoad() 
{
	//This function detects GPS errors that send the location to the wrong road.

	FVector VehicleLocation = Player->GetActorLocation();
	float PathLength = GetPathLengthToDestination(GPSList[0]);
	float DistanceFloat = FVector::Dist(VehicleLocation, GPSList[0]);
	if (PathLength >= DistanceFloat * 1.5f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Simworld => Wrong road"));
		return true;
	}
	return false;
}

bool ASimulatorWorld::DestinationIsOnLeftSideOfVehicle(FVector& Destination)
{
	FVector VehicleLocation = Player->GetActorLocation();
	FVector ForwardVector = Player->GetActorForwardVector();

	FVector ToDestination = (Destination - VehicleLocation).GetSafeNormal();

	// Calculate the cross product between the forward vector and the direction to the destination
	FVector CrossProduct = FVector::CrossProduct(ForwardVector, ToDestination);

	// Check the sign of the Z-component of the cross product
	if (CrossProduct.Z < 0)
	{
		// Destination is to the left
		UE_LOG(LogTemp, Log, TEXT("Destination is to the left of the vehicle"));
		return true;
	}
	return false;
}

void ASimulatorWorld::ResetAIMove() {
	IsFirstTimeMove = true;
	FirstMoveList.Empty();
}