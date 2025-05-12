// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "APIRunnable.h"
#include "APIService.generated.h"

UCLASS()
class MYPROJECT_API AAPIService : public AActor
{
    GENERATED_BODY()

public:
    AAPIService();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:

    UFUNCTION(BlueprintCallable)
    void StartService();

    UFUNCTION(BlueprintCallable)
    void StopService();

    UFUNCTION(BlueprintCallable)
    FString GetGPSData() const;

    UFUNCTION(BlueprintCallable)
    void PostNotification(bool InArea, int Fee, FString name);
private:
    TSharedPtr<FAPIRunnable> APIRunnable;
    FRunnableThread* Thread;
};
















//#include "CoreMinimal.h"
//#include "GameFramework/Actor.h"
//#include "HAL/Runnable.h"
//#include "HAL/RunnableThread.h"
//#include "HAL/CriticalSection.h"
//#include <Http.h>
//#include "APIService.generated.h"
//
//UCLASS()
//class MYPROJECT_API AAPIService : public AActor
//{
//    GENERATED_BODY()
//
//public:
//    AAPIService();
//
//    void StartThreadGetData();
//    void StopThreadGetData();
//
//    FString GetAPIUrlFromConfig() const;
//    FString GetLatestGPSData();
//    FString LatestGPSData = "0";
//    void PostANotification(bool InArea, int Fee, FString name);
//
//protected:
//    virtual void BeginPlay();
//    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);
//
//private:
//    FRunnableThread* Thread;
//    class FAPIRunnable* APIRunnable;
//    FCriticalSection Mutex;
//};
//
//class FAPIRunnable : public FRunnable
//{
//public:
//    FAPIRunnable(const FString& InAPIUrl);
//    //virtual ~FAPIRunnable();
//
//    virtual bool Init() override;
//    virtual uint32 Run() override;
//    virtual void Stop() override;
//
//    void GetDataFromAPI();
//    void PostNotification(bool InArea, int Fee, FString name);
//    FString GPSData = "0";
//
//    FString GetSafeGPSData();
//    FCriticalSection DataCriticalSection;
//
//private:
//    FHttpModule* Http;
//    bool bStopThread;
//    FString APIUrl;
//    AAPIService* APIService;
//};
