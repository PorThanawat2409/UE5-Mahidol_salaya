// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WBP_SaveNotification.generated.h"

UCLASS()
class MYPROJECT_API UWBP_SaveNotification : public UUserWidget
{
	GENERATED_BODY()
	
public:
    // Make sure to use the same name as your animation in the UMG editor
    UPROPERTY(meta = (BindWidgetAnim), Transient)
    UWidgetAnimation* ShowNotificationAnimation;

    // Optional: You can create a function to play the animation from C++
    UFUNCTION(BlueprintCallable)
    void PlayNotificationAnimation();

    UFUNCTION(BlueprintCallable)
    void PlayNotificationReverseAnimation();
};