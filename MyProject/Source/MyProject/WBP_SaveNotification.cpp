// Fill out your copyright notice in the Description page of Project Settings.


#include "WBP_SaveNotification.h"

void UWBP_SaveNotification::PlayNotificationAnimation()
{
    if (ShowNotificationAnimation)
    {
        PlayAnimation(ShowNotificationAnimation);
    }
}

void UWBP_SaveNotification::PlayNotificationReverseAnimation()
{
    if (ShowNotificationAnimation)
    {
        PlayAnimationReverse(ShowNotificationAnimation);
    }
}