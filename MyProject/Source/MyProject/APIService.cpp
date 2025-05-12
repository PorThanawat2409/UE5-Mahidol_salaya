// Fill out your copyright notice in the Description page of Project Settings.

#include "APIService.h"
#include "GameFramework/Actor.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include <Http.h>

AAPIService::AAPIService()
{
    PrimaryActorTick.bCanEverTick = true;
    APIRunnable = nullptr;
    Thread = nullptr;
}

void AAPIService::BeginPlay()
{
    Super::BeginPlay();
}

void AAPIService::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (Thread)
    {
        StopService();
    }
    Super::EndPlay(EndPlayReason);
}


void AAPIService::StartService()
{
    if (!Thread)
    {
        APIRunnable = MakeShared<FAPIRunnable>();
        Thread = FRunnableThread::Create(APIRunnable.Get(), TEXT("APIServiceThread"));

        UE_LOG(LogTemp, Warning, TEXT("APIServiceThread was created"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("APIServiceThread already created"));
    }
    //if (!Thread)
    //{
    //    APIRunnable = new FAPIRunnable();
    //    Thread = FRunnableThread::Create(APIRunnable, TEXT("APIServiceThread"));
    //    UE_LOG(LogTemp, Warning, TEXT("APIServiceThread was create"));
    //}
    //else
    //{
    //    UE_LOG(LogTemp, Warning, TEXT("APIServiceThread already created"));
    //}
}

void AAPIService::StopService()
{
    if (Thread)
    {
        APIRunnable->Stop();
        Thread->WaitForCompletion();
        
        delete Thread;
        Thread = nullptr;

        APIRunnable.Reset();

        UE_LOG(LogTemp, Warning, TEXT("APIServiceThread stopped and cleaned up"));
    }
    //if (Thread)
    //{
    //    if (APIRunnable)
    //    {
    //        APIRunnable->Stop();
    //    }

    //    Thread->WaitForCompletion();
    //    delete Thread;
    //    Thread = nullptr;

    //    delete APIRunnable;
    //    APIRunnable = nullptr;
    //}
}

FString AAPIService::GetGPSData() const
{
    if (APIRunnable)
    {
        return APIRunnable->GetGPSData();
    }

    return FString();
}


void AAPIService::PostNotification(bool InArea, int Fee, FString name) {
    APIRunnable->PostNotification(InArea, Fee, name);
}