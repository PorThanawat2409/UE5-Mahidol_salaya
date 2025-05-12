// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "WBP_Notification.generated.h"

UCLASS()
class MYPROJECT_API UWBP_Notification : public UUserWidget
{
	GENERATED_BODY()
	
public:
    UPROPERTY(meta = (BindWidgetAnim), Transient)
    UWidgetAnimation* ShowNotificationAnimation;

    UPROPERTY(meta = (BindWidgetAnim), Transient)
    UWidgetAnimation* ShowTopNotificationAnimation;

    UFUNCTION(BlueprintCallable)
    void ShowNotification(FText InputText);

    UFUNCTION(BlueprintCallable)
    void ShowTopNotification(FText InputText);

    UPROPERTY(meta = (BindWidget))
    UTextBlock* NotificationText;
};