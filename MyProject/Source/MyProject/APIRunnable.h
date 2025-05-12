// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include <Http.h>


class MYPROJECT_API FAPIRunnable : public FRunnable, public TSharedFromThis<FAPIRunnable>
{
public:
    FAPIRunnable();
    virtual ~FAPIRunnable();

    virtual bool Init() override;
    virtual uint32 Run() override;
    virtual void Stop() override;

    bool bCanFetchNewData = true;
    FString GetGPSData();

    void PostNotification(bool InArea, int Fee, FString name);
    FString GPSData;

private:
    FThreadSafeBool bIsRunning;

    FString APIUrl;

    FCriticalSection Mutex;

    void FetchDataFromAPI();

    TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> CurrentRequest;

    void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};