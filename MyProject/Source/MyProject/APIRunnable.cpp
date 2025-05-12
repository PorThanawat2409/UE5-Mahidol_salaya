// Fill out your copyright notice in the Description page of Project Settings.


#include "APIRunnable.h"
#include <Http.h>


FAPIRunnable::FAPIRunnable()
    : bIsRunning(true)
{
    FString ConfigFilePath = FPaths::Combine(*FPaths::ProjectContentDir(), TEXT("VirtualWorld/Config.ini"));
    GConfig->GetString(TEXT("Config"), TEXT("GoogleSheetAPI"), APIUrl, ConfigFilePath);
    APIUrl = "https://mongodbapi-v30h.onrender.com";
}

FAPIRunnable::~FAPIRunnable()
{
    Stop(); // Ensure we signal stop before destroying
}

bool FAPIRunnable::Init()
{
    // Initialization before the thread starts (if needed)
    return true;
}

uint32 FAPIRunnable::Run()
{
    while (true)
    {
        {
            FScopeLock Lock(&Mutex);
            if (!bIsRunning)
            {
                break;
            }
        }

        FetchDataFromAPI();
        FPlatformProcess::Sleep(1.0f); // Sleep for 1 second
    }

    return 0;
}

void FAPIRunnable::Stop()
{
    FScopeLock Lock(&Mutex);
    bIsRunning = false;

    if (CurrentRequest.IsValid())
    {
        CurrentRequest->CancelRequest();
    }
}

void FAPIRunnable::FetchDataFromAPI()
{
    if (!bCanFetchNewData)
    {
        UE_LOG(LogTemp, Warning, TEXT("Flag CanFetchNewData == false"));
        return;
    }

    if (APIUrl.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("GoogleSheetAPIURL is empty!"));
        return;
    }

    bCanFetchNewData = false;

    // Create an HTTP request
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    TWeakPtr<FAPIRunnable> LocalWeakThis = AsShared();

    Request->OnProcessRequestComplete().BindLambda([LocalWeakThis](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bWasSuccessful)
        {
            if (TSharedPtr<FAPIRunnable> Pinned = LocalWeakThis.Pin())
            {
                Pinned->OnResponseReceived(Req, Resp, bWasSuccessful);
            }
        });

    Request->SetURL(APIUrl + "/GetCurrentGPS");
    Request->SetVerb(TEXT("GET"));

    CurrentRequest = Request;

    Request->ProcessRequest();
}


void FAPIRunnable::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (!bIsRunning)
    {
        return;
    }

    if (bWasSuccessful && Response.IsValid())
    {
        const FString ResponseContent = Response->GetContentAsString();

        {
            FScopeLock Lock(&Mutex);
            GPSData = ResponseContent;
        }

        //UE_LOG(LogTemp, Log, TEXT("GPS Data received: %s"), *GPSData);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to receive response from API"));
    }
    bCanFetchNewData = true;
}

FString FAPIRunnable::GetGPSData()
{
    FScopeLock Lock(&Mutex);
    return GPSData;
}

void FAPIRunnable::PostNotification(bool InArea, int Fee, FString Name)
{
    UE_LOG(LogTemp, Log, TEXT("APIService PostNotification"));

    FString PostAPIUrl = APIUrl + TEXT("/AddNotification");

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(PostAPIUrl);
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    TSharedPtr<FJsonObject> RequestObj = MakeShared<FJsonObject>();

    if (InArea)
    {
        RequestObj->SetStringField(TEXT("status"), TEXT("yes"));
        RequestObj->SetStringField(TEXT("fee"), FString::FromInt(Fee));
        RequestObj->SetStringField(TEXT("name"), Name);
    }
    else
    {
        RequestObj->SetStringField(TEXT("status"), TEXT("no"));
        RequestObj->SetStringField(TEXT("fee"), FString::FromInt(Fee));
        RequestObj->SetStringField(TEXT("name"), TEXT(""));
    }

    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer);

    Request->SetContentAsString(RequestBody);

    Request->OnProcessRequestComplete().BindLambda([&](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bWasSuccessful)
        {
            if (bWasSuccessful && Resp.IsValid())
            {
                UE_LOG(LogTemp, Log, TEXT("Notification sent successfully: %s"), *Resp->GetContentAsString());
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to send notification"));
            }
        });

    Request->ProcessRequest();
}