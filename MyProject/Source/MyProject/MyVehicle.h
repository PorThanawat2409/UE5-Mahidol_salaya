// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
//#include "CarParameter.h"
#include <Components/TextRenderComponent.h>
#include <Components/PointLightComponent.h>
#include <Components/SpotLightComponent.h>
#include "MyChaosWVMovementComponent.h"
#include <Camera/CameraComponent.h>
#include "MyVehicleAIController.h"
#include "MyVehicle.generated.h"

UCLASS()
class MYPROJECT_API AMyVehicle : public AWheeledVehiclePawn
{
    GENERATED_BODY()

public:
    // Sets default values for this pawn's properties
    AMyVehicle();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    class ASimulatorWorld* SimWorld = nullptr;

    //UPROPERTY(BlueprintReadWrite, EditAnywhere)
    //class UCarParameter* CarParameter = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    class ACamera* Camera = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    class UMyChaosWVMovementComponent* ChaosVehicleMovementComponent = nullptr;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AMyVehicleAIController")
    class AMyVehicleAIController* VehicleAIController = nullptr;

    UPROPERTY(Category = Vehicle, VisibleDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    class UStaticMeshComponent* MeshComponent;

    UPROPERTY(Category = Vehicle, VisibleDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    class USkeletalMeshComponent* TRightSignal1;

    UPROPERTY(Category = Vehicle, VisibleDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    class USkeletalMeshComponent* TRightSignal2;

    UPROPERTY(Category = Vehicle, VisibleDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    class USkeletalMeshComponent* TRightSignal3;

    UPROPERTY(Category = Vehicle, VisibleDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    class USkeletalMeshComponent* TRightSignal4;


    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
    class USpringArmComponent* TSpringArm;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
    class USpringArmComponent* TSpringArm1;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
    class USpringArmComponent* TSpringArm2;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
    class USpringArmComponent* TSpringArm3;



    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
    class UPointLightComponent* TRightBackLight;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
    class UPointLightComponent* TLeftBackLight;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
    class UPointLightComponent* TLeftSideMarkerLight;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
    class UPointLightComponent* TRightSideMarkerLight;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
    class UPointLightComponent* TRightDRL;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
    class UPointLightComponent* TLeftDRL;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
    class UPointLightComponent* TLeftParking;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
    class UPointLightComponent* TRightParking;



    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
    class USpotLightComponent* TRightLowBeam;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
    class USpotLightComponent* TLeftLowBeam;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
    class USpotLightComponent* TLeftHighBeam;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Default")
    class USpotLightComponent* TRightHighBeam;



    UPROPERTY(EditAnywhere)
    UCameraComponent* TBackCamera;

    UPROPERTY(EditAnywhere)
    UCameraComponent* TTopCamera;

    UPROPERTY(EditAnywhere)
    UCameraComponent* TInCarCamera;

    UPROPERTY(EditAnywhere)
    UCameraComponent* TFrontCamera;

    UPROPERTY(EditAnywhere)
    UCameraComponent* TChaseCamera;

    UPROPERTY(EditAnywhere)
    UCameraComponent* TInternalCamera;



    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bIsInCar = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bRightTurnStatus = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bLeftTurnStatus = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bDRLStatus = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bDimmerStatus = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bLowbeamStatus = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bHightbeamStatus = false;

    bool OnGround() const;

    void HandleMovement(const FVector& Destination);


    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
    UCameraComponent* MyBackCamera;

private:
    // Steering input
    UFUNCTION(BlueprintCallable)
    void MoveRight(float Value);

    // Throttle input
    UFUNCTION(BlueprintCallable)
    void MoveForward(float Value);

    // Top view camera input setup
    UFUNCTION(BlueprintCallable)
    void TopViewCameraForward(float Value);
    UFUNCTION(BlueprintCallable)
    void TopViewCameraRight(float Value);
    UFUNCTION(BlueprintCallable)
    void TopViewCameraYaw(float Value);
    UFUNCTION(BlueprintCallable)
    void TopViewCameraPitch(float Value);
    UFUNCTION(BlueprintCallable)
    void TopViewCameraZoom(float Value);

    //Common Input Setup
    UFUNCTION(BlueprintCallable)
    void MouseLeftClick();
    UFUNCTION(BlueprintCallable)
    void ESC();

    // Placement preview input setup
    UFUNCTION(BlueprintCallable)
    void YawControlArea(float Value);
    UFUNCTION(BlueprintCallable)
    void PitchControlArea(float Value);
    UFUNCTION(BlueprintCallable)
    void ScaleWidthControlArea(float Value);
    UFUNCTION(BlueprintCallable)
    void ScaleLengthControlArea(float Value);

    // Handbrake and brake lights
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    class UInputAction* HandbrakeAction;
    UFUNCTION(BlueprintCallable)
    void IA_HandbrakeCompleted();
    UFUNCTION(BlueprintCallable)
    void IA_HandbrakeTriggered();

    // normal break
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    class UInputAction* BrakeAction;
    void ia_brakeTriggered();
    void ia_brakeStarted();
    void ia_brakeCompleted();

    // allows the player to rotate the camera around the car
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    class UInputAction* LookAround;
    void ia_lookaround();
    void interpstooriginalrotation(float DeltaTime);

    // toggle camera
    void internalcamera();

    // switches between back and front camera
    UFUNCTION(BlueprintCallable)
        void switchcamera();

    // resets the car in place - useful when you find yourself upside down
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    class UInputAction* Reset;
    void ia_reset();

    // rotate camera
    void mouseX(float Value);
    void mouseY(float Value);

    //  lighton input
    void rightturnsignal();
    void leftturnsignal();
    void drl();
    void dimmer();
    void lowbeam();
    void hightbeam();

    // customs function
    void BrakeLights(float value);


    void EnableIncarView(bool bIsInCar);

    void RightTurnSignal(bool Status);
    void LeftTurnSignal(bool Status);
    void ParkingLight(bool BrakeLights);
    void DRL(bool BrakeLights);
    void HeadLowBeam(bool BrakeLights);
    void HeadHighBeam(bool BrakeLights);

};
