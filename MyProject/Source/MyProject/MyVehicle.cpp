// Fill out your copyright notice in the Description page of Project Settings.


#include "MyVehicle.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "SimulatorWorld.h"
#include "Camera.h"
#include "Kismet/KismetMathLibrary.h"
#include <NavigationSystem.h>
#include "WheeledVehiclePawn.h"
#include "ChaosVehicleMovementComponent.h"
#include "MyChaosWVMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "MyVehicleAIController.h"
#include "Engine.h"

// Sets default values
AMyVehicle::AMyVehicle()
{
    
    // Set this pawn to call Tick() every frame.
    PrimaryActorTick.bCanEverTick = true;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);

    static ConstructorHelpers::FObjectFinder<UInputAction> HandbrakeActionObj(TEXT("/Game/VehicleTemplate/Input/Actions/IA_Handbrake.IA_Handbrake"));
    if (HandbrakeActionObj.Succeeded())
    {
        HandbrakeAction = HandbrakeActionObj.Object;
    }
    static ConstructorHelpers::FObjectFinder<UInputAction> BrakeActionObj(TEXT("/Game/VehicleTemplate/Input/Actions/IA_Break.IA_Break"));
    if (BrakeActionObj.Succeeded())
    {
        BrakeAction = BrakeActionObj.Object;
    }
    static ConstructorHelpers::FObjectFinder<UInputAction> ResetObj(TEXT("/Game/VehicleTemplate/Input/Actions/IA_Reset.IA_Reset"));
    if (ResetObj.Succeeded())
    {
        Reset = ResetObj.Object;
    }
    static ConstructorHelpers::FObjectFinder<UInputAction> LookAroundObj(TEXT("/Game/VehicleTemplate/Input/Actions/IA_LookAround.IA_LookAround"));
    if (LookAroundObj.Succeeded())
    {
        LookAround = LookAroundObj.Object;
    }

}

// Called when the game starts or when spawned
void AMyVehicle::BeginPlay()
{
    Super::BeginPlay();

    SimWorld = Cast<ASimulatorWorld>(UGameplayStatics::GetActorOfClass(GetWorld(), ASimulatorWorld::StaticClass()));
    Camera = Cast<ACamera>(UGameplayStatics::GetActorOfClass(GetWorld(), ACamera::StaticClass()));
    ChaosVehicleMovementComponent = Cast<UMyChaosWVMovementComponent>(GetComponentByClass(UMyChaosWVMovementComponent::StaticClass()));

   
    TArray<UPointLightComponent*> LightComponent;
    GetComponents<UPointLightComponent>(LightComponent);
    for (int32 i = 0, nb = LightComponent.Num(); i < nb; ++i) {
        if (LightComponent[i]->GetFName() == "RightBackLight") {
            TRightBackLight = LightComponent[i];
        }
        if (LightComponent[i]->GetFName() == "LeftBackLight") {
            TLeftBackLight = LightComponent[i];
        }
        if (LightComponent[i]->GetFName() == "LeftSideMarkerLight") {
            TLeftSideMarkerLight = LightComponent[i];
        }
        if (LightComponent[i]->GetFName() == "RightSideMarkerLight") {
            TRightSideMarkerLight = LightComponent[i];
        }
        if (LightComponent[i]->GetFName() == "RightDRL") {
            TRightDRL = LightComponent[i];
        }
        if (LightComponent[i]->GetFName() == "LeftDRL") {
            TLeftDRL = LightComponent[i];
        }
        if (LightComponent[i]->GetFName() == "LeftParking") {
            TLeftParking = LightComponent[i];
        }
        if (LightComponent[i]->GetFName() == "RightParking") {
            TRightParking = LightComponent[i];
        }
    }

    TArray<USpotLightComponent*> SpotLightComponent;
    GetComponents<USpotLightComponent>(SpotLightComponent);
    for (int32 i = 0, nb = SpotLightComponent.Num(); i < nb; ++i) {
        if (SpotLightComponent[i]->GetFName() == "RightLowBeam") {
            TRightLowBeam = SpotLightComponent[i];
        }
        if (SpotLightComponent[i]->GetFName() == "RightLowBeam") {
            TRightLowBeam = SpotLightComponent[i];
        }
        if (SpotLightComponent[i]->GetFName() == "LeftHighBeam") {
            TLeftHighBeam = SpotLightComponent[i];
        }
        if (SpotLightComponent[i]->GetFName() == "RightHighBeam") {
            TRightHighBeam = SpotLightComponent[i];
        }
    }

    TArray<USpringArmComponent*> SpringArmComponent;
    GetComponents<USpringArmComponent>(SpringArmComponent);
    for (int32 i = 0, nb = SpringArmComponent.Num(); i < nb; ++i) {
        if (SpringArmComponent[i]->GetFName() == "SpringArm") {
            TSpringArm = SpringArmComponent[i];
        }
        if (SpringArmComponent[i]->GetFName() == "SpringArm1") {
            TSpringArm1 = SpringArmComponent[i];
        }
        if (SpringArmComponent[i]->GetFName() == "SpringArm2") {
            TSpringArm2 = SpringArmComponent[i];
        }
        if (SpringArmComponent[i]->GetFName() == "SpringArm3") {
            TSpringArm3 = SpringArmComponent[i];
        }
    }

    TArray<UCameraComponent*> cameraComponents;
    GetComponents<UCameraComponent>(cameraComponents);
    for (UCameraComponent* CameraComponent : cameraComponents)
    {
        FName CameraName = CameraComponent->GetFName();
        UE_LOG(LogTemp, Log, TEXT("cameraComponents %s"), *CameraName.ToString());
        if (CameraName == "BackCamera")
        {
            TBackCamera = CameraComponent;

        }
        else if (CameraName == "TopCamera")
        {
            TTopCamera = CameraComponent;
        }
        else if (CameraName == "FrontCamera")
        {
            TFrontCamera = CameraComponent;
        }
        else if (CameraName == "InCarCamera")
        {
            TInCarCamera = CameraComponent;
        }
        else if (CameraName == "ChaseCamera")
        {
            TChaseCamera = CameraComponent;
        }
        else if (CameraName == "InternalCamera")
        {
            TInternalCamera = CameraComponent;
        }
    }

}

void AMyVehicle::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    bool bOnGround = ChaosVehicleMovementComponent->IsMovingOnGround();

    float SelectedValue = bOnGround ? 0.0f : 3.0f;
    
    MeshComponent->SetAngularDamping(SelectedValue);

    interpstooriginalrotation(DeltaTime);

}

void AMyVehicle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveForward", this, &AMyVehicle::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AMyVehicle::MoveRight);

    PlayerInputComponent->BindAxis("TopViewCameraForward", this, &AMyVehicle::TopViewCameraForward);
    PlayerInputComponent->BindAxis("TopViewCameraRight", this, &AMyVehicle::TopViewCameraRight);
    PlayerInputComponent->BindAxis("TopViewCameraYaw", this, &AMyVehicle::TopViewCameraYaw);
    PlayerInputComponent->BindAxis("TopViewCameraPitch", this, &AMyVehicle::TopViewCameraPitch);
    PlayerInputComponent->BindAxis("TopViewCameraZoom", this, &AMyVehicle::TopViewCameraZoom);

    PlayerInputComponent->BindAction("MouseLeftClick", IE_Pressed, this, &AMyVehicle::MouseLeftClick);
    PlayerInputComponent->BindAction("ESC", IE_Pressed, this, &AMyVehicle::ESC);

    PlayerInputComponent->BindAxis("YawControlArea", this, &AMyVehicle::YawControlArea);
    PlayerInputComponent->BindAxis("PitchControlArea", this, &AMyVehicle::PitchControlArea);
    PlayerInputComponent->BindAxis("ScaleWidthControlArea", this, &AMyVehicle::ScaleWidthControlArea);
    PlayerInputComponent->BindAxis("TopViewCamerZoom", this, &AMyVehicle::TopViewCameraZoom);
    PlayerInputComponent->BindAxis("ScaleLengthControlArea", this, &AMyVehicle::ScaleLengthControlArea);

    UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
    if (HandbrakeAction)
    {
        EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Triggered, this, &AMyVehicle::IA_HandbrakeTriggered);
        EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &AMyVehicle::IA_HandbrakeCompleted);
    }

    if (BrakeAction)
    {
        EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &AMyVehicle::ia_brakeTriggered);
        EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Started, this, &AMyVehicle::ia_brakeStarted);
        EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Completed, this, &AMyVehicle::ia_brakeCompleted);
    }

    PlayerInputComponent->BindAction("internalcamera", IE_Pressed, this, &AMyVehicle::internalcamera);
    PlayerInputComponent->BindAction("switchcamera", IE_Pressed, this, &AMyVehicle::switchcamera);

    if (Reset)
    {
        EnhancedInputComponent->BindAction(Reset, ETriggerEvent::Triggered, this, &AMyVehicle::ia_reset);
    }

    if (LookAround)
    {
        EnhancedInputComponent->BindAction(LookAround, ETriggerEvent::Triggered, this, &AMyVehicle::ia_lookaround);
    }

    PlayerInputComponent->BindAction("rightturnsignal", IE_Pressed, this, &AMyVehicle::rightturnsignal);
    PlayerInputComponent->BindAction("leftturnsignal", IE_Pressed, this, &AMyVehicle::leftturnsignal);
    PlayerInputComponent->BindAction("drl", IE_Pressed, this, &AMyVehicle::drl);
    PlayerInputComponent->BindAction("dimmer", IE_Pressed, this, &AMyVehicle::dimmer);
    PlayerInputComponent->BindAction("lowbeam", IE_Pressed, this, &AMyVehicle::lowbeam);
    PlayerInputComponent->BindAction("hightbeam", IE_Pressed, this, &AMyVehicle::hightbeam);

    PlayerInputComponent->BindAxis("LookUp", this, &AMyVehicle::mouseY);
    PlayerInputComponent->BindAxis("LookRight", this, &AMyVehicle::mouseX);
}

// Throttle input
void AMyVehicle::MoveForward(float Value)
{
    if (ChaosVehicleMovementComponent)
    {
        UE_LOG(LogTemp, Log, TEXT("MoveForward %f"), Value);
        ChaosVehicleMovementComponent->SetThrottleInput(Value);
        if (Value < 0.0f)
        {
            ChaosVehicleMovementComponent->SetTargetGear(-1, true);
        }
    }
    //if (!SimWorld)
    //{
    //    return;

    //}

    //if (!SimWorld->IsEditorMode)
    //{
    //    if (ChaosVehicleMovementComponent)
    //    {
    //        ChaosVehicleMovementComponent->SetThrottleInput(Value);
    //        if (Value < 0.0f)
    //        {
    //            ChaosVehicleMovementComponent->SetTargetGear(-1, true);
    //        }
    //    }
    //}
}


// Steering input
void AMyVehicle::MoveRight(float Value)
{
    if (ChaosVehicleMovementComponent)
    {
        ChaosVehicleMovementComponent->SetSteeringInput(Value);
    }

    //if (!SimWorld)
    //{
    //    return;
    //}

    //if (!SimWorld->IsEditorMode)
    //{
    //    if (ChaosVehicleMovementComponent)
    //    {
    //        ChaosVehicleMovementComponent->SetSteeringInput(Value);
    //    }
    //}
}

// Top view camera input setup
void AMyVehicle::TopViewCameraForward(float Value)
{

    if (Camera)
    {
        Camera->ForwardTopViewCamera(Value);
    }

    if (!SimWorld)
    {
        return;
    }

    if (SimWorld->IsEditorMode)
    {
        if (Camera)
        {
            Camera->ForwardTopViewCamera(Value);
        }
    }
}

void AMyVehicle::TopViewCameraRight(float Value)
{

    if (Camera)
    {
        Camera->RightTopViewCamera(Value);
    }

    if (!SimWorld)
    {
        return;
    }

    if (SimWorld->IsEditorMode)
    {
        if (Camera)
        {
            Camera->RightTopViewCamera(Value);
        }
    }
}

void AMyVehicle::TopViewCameraYaw(float Value)
{
    if (Camera)
    {
        Camera->YawTopViewCamera(Value);
    }

    if (!SimWorld)
    {
        return;
    }

    if (SimWorld->IsEditorMode)
    {
        if (Camera)
        {
            Camera->YawTopViewCamera(Value);
        }
    }
}

void AMyVehicle::TopViewCameraPitch(float Value)
{
    if (Camera)
    {
        Camera->PitchTopViewCamera(Value);
    }

    if (!SimWorld)
    {
        return;
    }

    if (SimWorld->IsEditorMode)
    {
        if (Camera)
        {
            Camera->PitchTopViewCamera(Value);
        }
    }
}

void AMyVehicle::TopViewCameraZoom(float Value)
{
    if (!SimWorld)
    {
        return;
    }

    if (SimWorld->IsEditorMode)
    {
        if (Camera)
        {
            Camera->ZoomTopViewCamera(Value);
        }
    }
}

//Common Input Setup
void AMyVehicle::MouseLeftClick()
{
    if (!SimWorld)
    {
        return;
    }

    if (SimWorld)
    {
        SimWorld->OnMouseLeftClick();
    }
}

void AMyVehicle::ESC()
{
    if (!SimWorld)
    {
        return;
    }

    if (SimWorld)
    {
        SimWorld->OnESCKey();
    }
}

// Placement preview input setup
void AMyVehicle::YawControlArea(float Value)
{
    if (!SimWorld)
    {
        return;
    }

    if (SimWorld->IsEditorMode)
    {
        SimWorld->YawRotatePlacementPreview(Value);
    }
}

void AMyVehicle::PitchControlArea(float Value)
{
    if (!SimWorld)
    {
        return;
    }

    if (SimWorld->IsEditorMode)
    {
        SimWorld->PitchRotatePlacementPreview(Value);
    }
}

void AMyVehicle::ScaleWidthControlArea(float Value)
{
    if (!SimWorld)
    {
        return;
    }

    if (SimWorld->IsEditorMode)
    {
        SimWorld->ScaleWidthPlacementPreview(Value);
    }
}

void AMyVehicle::ScaleLengthControlArea(float Value)
{
    if (!SimWorld)
    {
        return;
    }

    if (SimWorld->IsEditorMode)
    {
        SimWorld->ScaleLengthPlacementPreview(Value);
    }
}


// Handbrake and brake lights
void AMyVehicle::IA_HandbrakeTriggered()
{
    ChaosVehicleMovementComponent->SetHandbrakeInput(true);
    BrakeLights(true);
    //if (!SimWorld)
    //{
    //    return;
    //}
    //if (!SimWorld->IsEditorMode)
    //{
    //    ChaosVehicleMovementComponent->SetHandbrakeInput(true);
    //    BrakeLights(true);
    //}
}
void AMyVehicle::IA_HandbrakeCompleted()
{
    ChaosVehicleMovementComponent->SetHandbrakeInput(false);
    BrakeLights(false);
    //if (!SimWorld)
    //{
    //    return;
    //}

    //if (!SimWorld->IsEditorMode)
    //{
    //    ChaosVehicleMovementComponent->SetHandbrakeInput(false);
    //    BrakeLights(false);
    //}
}


// normal break
void AMyVehicle::ia_brakeTriggered()
{
    ChaosVehicleMovementComponent->SetBrakeInput(1.0f);

    //if (!SimWorld)
    //{
    //    return;
    //}
    //if (!SimWorld->IsEditorMode)
    //{
    //    ChaosVehicleMovementComponent->SetBrakeInput(1.0f);
    //}
}

void AMyVehicle::ia_brakeStarted()
{
    BrakeLights(true);

    //if (!SimWorld)
    //{
    //    return;
    //}
    //if (!SimWorld->IsEditorMode)
    //{
    //    BrakeLights(true);
    //}
}   

void AMyVehicle::ia_brakeCompleted()
{
    BrakeLights(false);
    ChaosVehicleMovementComponent->SetBrakeInput(0.0f);
    //if (!SimWorld)
    //{
    //    return;
    //}
    //if (!SimWorld->IsEditorMode)
    //{
    //    BrakeLights(false);
    //    ChaosVehicleMovementComponent->SetBrakeInput(0.0f);
    //}
}

// allows the player to rotate the camera around the car
void AMyVehicle::ia_lookaround()
{
    TSpringArm->AddLocalRotation(FRotator(0, 1.0f, 0));
}

void AMyVehicle::interpstooriginalrotation(float deltatime)
{
    FRotator currentrotation = TSpringArm->GetRelativeRotation();

    FRotator newrotation = FMath::RInterpTo(currentrotation, FRotator::ZeroRotator, deltatime, 1.0f);

    TSpringArm->SetRelativeRotation(newrotation);
}


// toggle camera
void AMyVehicle::internalcamera()
{
    bIsInCar = !bIsInCar;
    EnableIncarView(bIsInCar);

}


// switches between back and front camera
void AMyVehicle::switchcamera()
{
    if (TBackCamera->IsActive()) {
        TBackCamera->Deactivate();
        TTopCamera->Activate();
    }
    else {
        if (TTopCamera->IsActive()) {
            TTopCamera->Deactivate();
            TFrontCamera->Activate();
        }
        else {
            TFrontCamera->Deactivate();
            TBackCamera->Activate();
        }
    }
}


// resets the car in place - useful when you find yourself upside down
void AMyVehicle::ia_reset()
{
    FVector Location = GetActorLocation();
    Location.Z += 50.f;

    FRotator Rotation = GetActorRotation();
    Rotation.Pitch = 0.f;
    Rotation.Roll = 0.f;

    FVector Scale(1.f, 1.f, 1.f);

    FTransform NewTransform(Rotation, Location, Scale);

    SetActorTransform(NewTransform, false, nullptr, ETeleportType::TeleportPhysics);

    if (MeshComponent)
    {
        MeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
        MeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("MeshComponent is not assigned!"));
    }
}


// rotate camera
void AMyVehicle::mouseX(float Value)
{
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PlayerController->IsInputKeyDown(EKeys::RightMouseButton)) {
        if (TBackCamera->IsActive()) {
            TSpringArm->AddWorldRotation(FRotator(0, Value*2, 0));
        }
        else if (TBackCamera->IsActive() == false) {
            if (TTopCamera->IsActive()) {
                TSpringArm2->AddWorldRotation(FRotator(0, Value*2, 0));
            }
        }
    }
}

void AMyVehicle::mouseY(float Value)
{

    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PlayerController->IsInputKeyDown(EKeys::RightMouseButton)) {
        if (TBackCamera->IsActive()) {
            TSpringArm->AddLocalRotation(FRotator(Value*2, 0, 0));
        }
        else if (TBackCamera->IsActive() == false) {
            if (TTopCamera->IsActive()) {
                TSpringArm2->AddLocalRotation(FRotator(Value*2, 0, 0));
            }
        }
    }
}


//  lighton input
void AMyVehicle::rightturnsignal()
{
    bRightTurnStatus = !bRightTurnStatus;
}

void AMyVehicle::leftturnsignal()
{
    bLeftTurnStatus = !bLeftTurnStatus;
}

void AMyVehicle::drl()
{
    bDRLStatus = !bDRLStatus;
}

void AMyVehicle::dimmer()
{
    bDimmerStatus = !bDimmerStatus;
}

void AMyVehicle::lowbeam()
{
    bLowbeamStatus = !bLowbeamStatus;
}

void AMyVehicle::hightbeam()
{
    bHightbeamStatus = !bHightbeamStatus;
}



void AMyVehicle::BrakeLights(float value)
{
    FString MaterialPath = TEXT("/Game/Vehicles/CommonMaterial/Glass_red_light.Glass_red_light");
    // Load the material from the specified path
    UMaterialInterface* GlassRedLightMaterial = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *MaterialPath));
    UMaterialInstanceDynamic* DynamicMaterialInstance1 = UMaterialInstanceDynamic::Create(GlassRedLightMaterial, this);
    if (DynamicMaterialInstance1)
    {
        MeshComponent->SetMaterial(36, DynamicMaterialInstance1);
        if (value)
        {
            DynamicMaterialInstance1->SetScalarParameterValue("EmissiveIntensity", 0.1f);
            TRightBackLight->SetIntensity(1000.0f);
        }
        else
        {
            DynamicMaterialInstance1->SetScalarParameterValue("EmissiveIntensity", 0.0f);
            TRightBackLight->SetIntensity(0.0f);
        }
    }

    UMaterialInstanceDynamic* DynamicMaterialInstance2 = UMaterialInstanceDynamic::Create(GlassRedLightMaterial, this);
    if (DynamicMaterialInstance2)
    {
        MeshComponent->SetMaterial(37, DynamicMaterialInstance2);
        if (value)
        {
            DynamicMaterialInstance2->SetScalarParameterValue("EmissiveIntensity", 0.1f);
            TLeftBackLight->SetIntensity(1000.0f);
        }
        else
        {
            DynamicMaterialInstance2->SetScalarParameterValue("EmissiveIntensity", 0.0f);
            TLeftBackLight->SetIntensity(0.0f);
        }
    }
}

void AMyVehicle::EnableIncarView(bool IsInCar)
{
    if (IsInCar) {
        TTopCamera->Activate();
        TBackCamera->Deactivate();
    }

    else {
        TTopCamera->Deactivate();
        TBackCamera->Activate();
    }
}

void AMyVehicle::RightTurnSignal(bool Status) {


    if (Status) {
        FString TurnSignalPath = TEXT("/Game/Vehicles/SportsCar/Materials/TurnSignal_On.TurnSignal_On");
        UMaterialInterface* TurnSignalMaterial = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *TurnSignalPath));
        UMaterialInstanceDynamic* TurnSignal_On = UMaterialInstanceDynamic::Create(TurnSignalMaterial, this);
        TRightSignal1->SetMaterial(0, TurnSignal_On);
        TRightSignal2->SetMaterial(0, TurnSignal_On);
        TRightSignal3->SetMaterial(0, TurnSignal_On);
        TRightSignal4->SetMaterial(0, TurnSignal_On);
    }
    else {
        FString M_SportscarlPath = TEXT("/Game/Vehicles/SportsCar/Materials/M_Sportscar_SideMarkerLight.M_Sportscar_SideMarkerLight");
        UMaterialInterface* M_SportscarlMaterial = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *M_SportscarlPath));
        UMaterialInstanceDynamic* M_Sportscar_SideMarkerLight = UMaterialInstanceDynamic::Create(M_SportscarlMaterial, this);
        TRightSignal1->SetMaterial(0, M_Sportscar_SideMarkerLight);
        TRightSignal2->SetMaterial(0, M_Sportscar_SideMarkerLight);
        TRightSignal3->SetMaterial(0, M_Sportscar_SideMarkerLight);
        TRightSignal4->SetMaterial(0, M_Sportscar_SideMarkerLight);
    }
}

void AMyVehicle::LeftTurnSignal(bool Status) {
    if (Status) {
        FString TurnSignalPath = TEXT("/Game/Vehicles/SportsCar/Materials/TurnSignal_On.TurnSignal_On");
        UMaterialInterface* TurnSignalMaterial = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *TurnSignalPath));
        UMaterialInstanceDynamic* TurnSignal_On = UMaterialInstanceDynamic::Create(TurnSignalMaterial, this);
        TRightSignal1->SetMaterial(0, TurnSignal_On);
        TRightSignal2->SetMaterial(0, TurnSignal_On);
        TRightSignal3->SetMaterial(0, TurnSignal_On);
        TRightSignal4->SetMaterial(0, TurnSignal_On);
    }
    else {
        FString M_SportscarlPath = TEXT("/Game/Vehicles/SportsCar/Materials/M_Sportscar_SideMarkerLight.M_Sportscar_SideMarkerLight");
        UMaterialInterface* M_SportscarlMaterial = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *M_SportscarlPath));
        UMaterialInstanceDynamic* M_Sportscar_SideMarkerLight = UMaterialInstanceDynamic::Create(M_SportscarlMaterial, this);
        TRightSignal1->SetMaterial(0, M_Sportscar_SideMarkerLight);
        TRightSignal2->SetMaterial(0, M_Sportscar_SideMarkerLight);
        TRightSignal3->SetMaterial(0, M_Sportscar_SideMarkerLight);
        TRightSignal4->SetMaterial(0, M_Sportscar_SideMarkerLight);
    }
}

void AMyVehicle::ParkingLight(bool BrakeLights) {
    FString M_SportscarlPath = TEXT("/Game/Vehicles/SportsCar/Materials/M_Sportscar_SideMarkerLight.M_Sportscar_SideMarkerLight");
    UMaterialInterface* M_SportscarlMaterial = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *M_SportscarlPath));
    UMaterialInstanceDynamic* M_Sportscar_SideMarkerLight = UMaterialInstanceDynamic::Create(M_SportscarlMaterial, this);
    UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(M_Sportscar_SideMarkerLight, this);
    MeshComponent->SetMaterial(0, DynMaterial);
    if (BrakeLights) {
        DynMaterial->SetScalarParameterValue("LeftEmission", 500.0f);
    }
    else {
        DynMaterial->SetScalarParameterValue("LeftEmission", 2.0f);
    }
}

void AMyVehicle::DRL(bool BrakeLights) {
    FString M_SportscarlPath = TEXT("/Game/Vehicles/SportsCar/Materials/M_Sportscar_SideMarkerLight.M_Sportscar_SideMarkerLight");
    UMaterialInterface* M_SportscarlMaterial = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *M_SportscarlPath));
    UMaterialInstanceDynamic* M_Sportscar_SideMarkerLight = UMaterialInstanceDynamic::Create(M_SportscarlMaterial, this);
    UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(M_Sportscar_SideMarkerLight, this);
    MeshComponent->SetMaterial(0, DynMaterial);
    if (BrakeLights) {
        DynMaterial->SetScalarParameterValue("DRLPower", 500.0f);
        TRightDRL->SetIntensity(200.0f);
        TLeftDRL->SetIntensity(200.0f);
    }
    else {
        DynMaterial->SetScalarParameterValue("DRLPower", 2.0f);
        TRightDRL->SetIntensity(0);
        TLeftDRL->SetIntensity(0);
    }
}

void AMyVehicle::HeadLowBeam(bool BrakeLights) {
    FString M_SportscarlPath = TEXT("/Game/Vehicles/SportsCar/Materials/M_Sportscar_SideMarkerLight.M_Sportscar_SideMarkerLight");
    UMaterialInterface* M_SportscarlMaterial = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *M_SportscarlPath));
    UMaterialInstanceDynamic* M_Sportscar_SideMarkerLight = UMaterialInstanceDynamic::Create(M_SportscarlMaterial, this);
    UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(M_Sportscar_SideMarkerLight, this);
    MeshComponent->SetMaterial(0, DynMaterial);
    if (BrakeLights) {
        DynMaterial->SetScalarParameterValue("HeadLightPower", 200.0f);
        TLeftLowBeam->SetIntensity(200000.0f);
        TRightLowBeam->SetIntensity(200000.0f);
    }
    else {
        DynMaterial->SetScalarParameterValue("HeadLightPower", 2.0f);
        TLeftLowBeam->SetIntensity(0);
        TRightLowBeam->SetIntensity(0);
    }
}

void AMyVehicle::HeadHighBeam(bool BrakeLights) {
    FString M_SportscarlPath = TEXT("/Game/Vehicles/SportsCar/Materials/M_Sportscar_SideMarkerLight.M_Sportscar_SideMarkerLight");
    UMaterialInterface* M_SportscarlMaterial = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *M_SportscarlPath));
    UMaterialInstanceDynamic* M_Sportscar_SideMarkerLight = UMaterialInstanceDynamic::Create(M_SportscarlMaterial, this);
    UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(M_Sportscar_SideMarkerLight, this);
    MeshComponent->SetMaterial(0, DynMaterial);
    if (BrakeLights) {
        DynMaterial->SetScalarParameterValue("HeadLightPower", 200.0f);
        TLeftLowBeam->SetIntensity(200.0f);
        TRightLowBeam->SetIntensity(200.0f);
    }
    else {
        DynMaterial->SetScalarParameterValue("HeadLightPower", 2.0f);
        TLeftLowBeam->SetIntensity(0);
        TRightLowBeam->SetIntensity(0);
    }
}

void AMyVehicle::HandleMovement(const FVector& Destination)
{
    // Get the player controller
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PlayerController && PlayerController->GetPawn() == this)
    {
        PlayerController->UnPossess();
        UE_LOG(LogTemp, Warning, TEXT("Player controller unpossessed the vehicle"));
    }


    // Ensure the vehicle has a valid AI controller
    VehicleAIController = Cast<AMyVehicleAIController>(UGameplayStatics::GetActorOfClass(GetWorld(), AMyVehicleAIController::StaticClass()));
    if (!VehicleAIController)
    {
        VehicleAIController = GetWorld()->SpawnActor<AMyVehicleAIController>(AMyVehicleAIController::StaticClass());

        UE_LOG(LogTemp, Warning, TEXT("No AI Controller found, attempting to spawn and possess one"));
        if (VehicleAIController->GetPawn() != this)
        {

            VehicleAIController->Possess(this);
            UE_LOG(LogTemp, Warning, TEXT("AI Controller spawned and possessed the vehicle"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to spawn AI Controller"));
            return;
        }
    }

    if (VehicleAIController->GetPawn() != this)
    {

        VehicleAIController->Possess(this);
        UE_LOG(LogTemp, Warning, TEXT("AI Controller spawned and possessed the vehicle"));

    }

    if (VehicleAIController)
    {
        VehicleAIController->MoveToTargetLocation(Destination);
    }
}

//Cheack vehicle is on ground
bool AMyVehicle::OnGround() const
{
    FVector Start = GetActorLocation();
    FVector End = Start - FVector(0.0f, 0.0f, 10.0f);  // Check 10 units below the vehicle

    FHitResult HitResult;
    FCollisionQueryParams CollisionParams;

    // Perform a line trace (raycast) to detect if there is something between Start and End points
    bool bIsOnGround = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECC_Visibility,
        CollisionParams
    );

    return bIsOnGround;
}