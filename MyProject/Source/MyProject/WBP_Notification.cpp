// Fill out your copyright notice in the Description page of Project Settings.


#include "WBP_Notification.h"

void UWBP_Notification::ShowNotification(FText InputText) {

    if (NotificationText)  
    {
        NotificationText->SetText(InputText);
    }
    if (ShowNotificationAnimation)
    {
        PlayAnimation(ShowNotificationAnimation);
    }
}

void UWBP_Notification::ShowTopNotification(FText InputText) {

    if (NotificationText)  
    {
        NotificationText->SetText(InputText);
    }
    if (ShowTopNotificationAnimation)
    {
        PlayAnimation(ShowTopNotificationAnimation);
    }
}