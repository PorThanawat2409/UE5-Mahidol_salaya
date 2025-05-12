// Fill out your copyright notice in the Description page of Project Settings.


#include "MahidolPlayerController.h"
#include "MyVehicle.h"
#include "APIService.h"
#include "Engine.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"
//#include <GeographicCoordinates.h>

AMahidolPlayerController::AMahidolPlayerController()
{
	// Load the InputMappingContext asset at compile time
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> MappingContextFinder(TEXT("/Game/VehicleTemplate/Input/IMC_Default.IMC_Default"));
	if (MappingContextFinder.Object != NULL)
	{
		InputMappingContext = MappingContextFinder.Object;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load InputMappingContext asset!"));
	}
}

void AMahidolPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Bind the ToggleMinimap function to the "ToggleMinimap" action
	InputComponent->BindAction("ToggleMinimap", IE_Pressed, this, &AMahidolPlayerController::ToggleMinimap);

}

void AMahidolPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Find and cast the Vehicle Actor
	VehicleActor = Cast<AMyVehicle>(UGameplayStatics::GetActorOfClass(GetWorld(), AMyVehicle::StaticClass()));
	// Find and cast the API Service
	APIService = Cast<AAPIService>(UGameplayStatics::GetActorOfClass(GetWorld(), AAPIService::StaticClass()));
	// Find and cast the GeoReferencingSystem
	geo = Cast<AGeoReferencingSystem>(UGameplayStatics::GetActorOfClass(GetWorld(), AGeoReferencingSystem::StaticClass()));
	// Find the Player Actor
	for (TActorIterator<APawn> It(GetWorld()); It; ++It)
	{
		if (It->GetName().Contains(TEXT("BP_Character_TTBoyB")))
		{
			PlayerActor = *It;
			break;
		}
	}

	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

		if (Subsystem && InputMappingContext)
		{
			Subsystem->AddMappingContext(InputMappingContext, 0);
		}
	}
	MinimapWidgetClass = StaticLoadClass(UUserWidget::StaticClass(), nullptr, TEXT("/Game/Mahidol_Map/MinimapMahidol/WBP_MahidolMinimap.WBP_MahidolMinimap_C"));
	if (MinimapWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("MinimapWidgetClass successfully loaded"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load MinimapWidgetClass"));
	}

}

void AMahidolPlayerController::Tick(float DeltaTime)
{	
	Super::Tick(DeltaTime);
	if (IsEnableMoveToGPS)
	{
		MoveActorToLocation();
	}
	else {
		if ((GetPawn() != PlayerActor) && (IsPlayerActor == true))
		{
			this->Possess(PlayerActor);
		}
	}
}

void AMahidolPlayerController::SwitchActor()
{
	if (IsPlayerActor) 
	{
		if (PlayerActor)
		{
			// Get PlayerActor location
			PlayerLocation = PlayerActor->GetActorLocation(); 
			PlayerRotation = PlayerActor->GetActorRotation();

			// Hide PlayerActor
			PlayerActor->SetActorHiddenInGame(true);

			// Disable collision for PlayerActor
			PlayerActor->SetActorEnableCollision(false);
		}

		if (VehicleActor)
		{
			// Show VehicleActor
			VehicleActor->SetActorHiddenInGame(false);

			// Enable physics simulation for VehicleActor
			UPrimitiveComponent* VehicleRoot = Cast<UPrimitiveComponent>(VehicleActor->GetRootComponent());
			if (VehicleRoot)
			{
				VehicleRoot->SetSimulatePhysics(true);
			}

			// Enable collision for VehicleActor
			VehicleActor->SetActorEnableCollision(true);

			// Set VehicleActor location to the last PlayerActor location
			VehicleActor->SetActorLocation(PlayerLocation, false, nullptr, ETeleportType::TeleportPhysics);
			VehicleActor->SetActorRotation(PlayerRotation, ETeleportType::TeleportPhysics);

			// Possess the VehicleActor
			this->Possess(VehicleActor);
		}
		
		IsPlayerActor = false;
	}
	else
	{
		if (VehicleActor)
		{
			// Get VehicleActor location
			VehicleLocation = VehicleActor->GetActorLocation();
			VehicleRotation = VehicleActor->GetActorRotation();
			VehicleRotation.Pitch = 0.0f;
			VehicleRotation.Roll = 0.0f;
			VehicleLocation.Z += 100;
			// Hide VehicleActor
			VehicleActor->SetActorHiddenInGame(true);

			// Disable collision for VehicleActor
			VehicleActor->SetActorEnableCollision(false);

			// Disable physics simulation for VehicleActor
			UPrimitiveComponent* VehicleRoot = Cast<UPrimitiveComponent>(VehicleActor->GetRootComponent());
			if (VehicleRoot)
			{
				VehicleRoot->SetSimulatePhysics(false);
			}
		}

		if (PlayerActor)
		{
			// Show PlayerActor
			PlayerActor->SetActorHiddenInGame(false);

			// Enable collision for PlayerActor
			PlayerActor->SetActorEnableCollision(true);
			
			// Set PlayerActor location to the last VehicleActor location
			PlayerActor->SetActorLocation(VehicleLocation);
			PlayerActor->SetActorRotation(VehicleRotation);

			// Possess the PlayerActor
			this->Possess(PlayerActor);
		}

		
		IsPlayerActor = true;
	}
}

FVector AMahidolPlayerController::GetCloseLocationOnNavMesh(FVector& Destination)
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

			FVector NewLocation = ProjectedLocation.Location;
			return NewLocation;
		}
	}
	return Destination;
}

void AMahidolPlayerController::ToggleMinimap()
{
	if (!MinimapWidget && MinimapWidgetClass)
	{
		// Create the widget the first time it's needed
		MinimapWidget = CreateWidget<UUserWidget>(this, MinimapWidgetClass);

		if (MinimapWidget)
		{
			MinimapWidget->AddToViewport(3);
			MinimapWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (MinimapWidget)
	{
		// Toggle visibility
		if (MinimapWidget->GetVisibility() == ESlateVisibility::Visible)
		{
			MinimapWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			MinimapWidget->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void AMahidolPlayerController::MoveActorToLocation()
{
	float MaxWalkSpeed = 2000.0f;

	FVector targetLocation = GetDestination();

	if (IsFirstTimeMove)
	{
		if (IsCompletedGetGPS && IsPlayerActor)
		{
			targetLocation.Z += 10;
			//LastLocation = targetLocation;
			PlayerActor->SetActorLocation(targetLocation);
			IsFirstTimeMove = false;
		}
		else if (IsCompletedGetGPS && !IsPlayerActor)
		{
			VehicleActor->SetActorLocation(targetLocation, false, nullptr, ETeleportType::TeleportPhysics);
			IsFirstTimeMove = false;
		}
	}
	else if (IsPlayerActor)
	{
		// get ai controller 
		AAIController* AIController = Cast<AAIController>(UGameplayStatics::GetActorOfClass(GetWorld(), AAIController::StaticClass()));
		if (!AIController)
		{
			UE_LOG(LogTemp, Warning, TEXT("AIController is not valid!"));
			return;
		}

		this->UnPossess();
		if (GetViewTarget() != PlayerActor) {
			SetViewTargetWithBlend(PlayerActor);
		}
		
		// Set movement component max walk speed for PlayerActor
		UCharacterMovementComponent* MovementComponent = Cast<UCharacterMovementComponent>(PlayerActor);
		if (MovementComponent)
		{
			MovementComponent->MaxWalkSpeed = MaxWalkSpeed;
		}

		// Check if the AIController is already possessing the PlayerActor
		if (AIController->GetPawn() != PlayerActor)
		{
			// Unpossess any current pawn the AIController is managing
			AIController->UnPossess();

			// Possess the PlayerActor
			AIController->Possess(PlayerActor);
		}


		// ai move to PlayerActor to targetLocation
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalLocation(targetLocation);
		MoveRequest.SetAcceptanceRadius(10.0f);

		FNavPathSharedPtr NavPath;
		
		DistanceToLastTarget = FVector::Dist(PlayerActor->GetActorLocation(), targetLocation);
		if (DistanceToLastTarget >= 5000) {
			IsFirstTimeMove = true;
		}

		EPathFollowingRequestResult::Type MoveResult = AIController->MoveToLocation(targetLocation, 50.0f, true, true, false, true, 0, true);

		if (MoveResult == EPathFollowingRequestResult::Failed)
		{
			UE_LOG(LogTemp, Warning, TEXT("AI MoveTo request failed."));
		}
		else if (MoveResult == EPathFollowingRequestResult::AlreadyAtGoal)
		{
			//UE_LOG(LogTemp, Log, TEXT("PlayerActor is already at the target location."));
		}
	}
	else if (!IsPlayerActor)
	{
		VehicleActor->HandleMovement(targetLocation);
		if (GetViewTarget() != VehicleActor) {
			SetViewTargetWithBlend(VehicleActor);
		}
	}
	
}

FVector AMahidolPlayerController::GetDestination()
{
	if (APIService)
	{
		TSharedPtr<FJsonObject> JsonObject;
		FString GPSData = APIService->GetGPSData();
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(GPSData);
		if (FJsonSerializer::Deserialize(Reader, JsonObject))
		{
			TargetLatitude = JsonObject->GetStringField("lat");
			TargetLongtitude = JsonObject->GetStringField("long");
			IsCompletedGetGPS = true;
		}
		else {
			UE_LOG(LogTemp, Log, TEXT("Unable to Fetch data yet"));
			IsCompletedGetGPS = false;
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Unable to use APIService"));
		IsCompletedGetGPS = false;
	}

	FGeographicCoordinates geoStruct;
	FVector worldLocation;
	FVector targetLocation;
	
	geoStruct.Altitude = 10;
	geoStruct.Latitude = FCString::Atod(*TargetLatitude);
	geoStruct.Longitude = FCString::Atod(*TargetLongtitude);
	geo->GeographicToECEF(geoStruct, worldLocation);
	geo->ECEFToEngine(worldLocation, targetLocation);

	targetLocation = GetCloseLocationOnNavMesh(targetLocation);
	//DrawDebugSphere(GetWorld(), targetLocation, 100.0f, 5, FColor::Blue, false, 2.0f);

	return targetLocation;
}

void AMahidolPlayerController::PostStatusToServer(bool InArea, int Fee, FString Name)
{
	if (APIService)
	{
		APIService->PostNotification(InArea, Fee, Name);
	}
}
