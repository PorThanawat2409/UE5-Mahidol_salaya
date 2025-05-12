// Fill out your copyright notice in the Description page of Project Settings.


#include "GM_Mahidol.h"
#include "GameFramework/HUD.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"
#include "APIService.h"
#include <LevelSequence/Public/LevelSequence.h>
#include <LevelSequence/Public/LevelSequenceActor.h>
#include <LevelSequence/Public/LevelSequencePlayer.h>
#include "CineCameraActor.h"
#include "Blueprint/UserWidget.h"
#include <Kismet/GameplayStatics.h>
#include "Engine.h"
#include "MahidolPlayerController.h"

AGM_Mahidol::AGM_Mahidol()
{

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/OneToonTeen/Blueprints/Characters/BP_Character_TTBoyB"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// set default HUD class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<AHUD> MahidolHUDBPClass(TEXT("/Game/Mahidol_Map/Widget_UI/Mahidol_HUD"));
	if (MahidolHUDBPClass.Class != NULL)
	{
		HUDClass = MahidolHUDBPClass.Class;
	}
	PlayerControllerClass = AMahidolPlayerController::StaticClass();
}

void AGM_Mahidol::BeginPlay()
{
	Super::BeginPlay();
	APIService = Cast<AAPIService>(UGameplayStatics::GetActorOfClass(GetWorld(), AAPIService::StaticClass()));
	PlayerController = Cast<AMahidolPlayerController>(UGameplayStatics::GetActorOfClass(GetWorld(), AMahidolPlayerController::StaticClass()));
	// Find the WBP_MainUI
	if (!WBP_MainUI)
	{
		WBP_MainUI = StaticLoadClass(UUserWidget::StaticClass(), nullptr, TEXT("/Game/Mahidol_Map/Widget_UI/WBP_MainUI.WBP_MainUI_C"));
	}
	if (WBP_MainUI)
	{
		// Create the widget instance
		WidgetInstance_MainUI = CreateWidget<UUserWidget>(GetWorld(), WBP_MainUI);
		if (WidgetInstance_MainUI)
		{
			// Add the widget to the viewport
			WidgetInstance_MainUI->AddToViewport(2);

			UE_LOG(LogTemp, Warning, TEXT("Widget successfully created and added to viewport!"));
		}
	}
	
	if (PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerController was found!"));
	}
	if (PlayerController && PlayerController->InputComponent)
	{
		PlayerController->InputComponent->BindAction("PlayStartSequence", IE_Pressed, this, &AGM_Mahidol::PlayStartSequence);
		UE_LOG(LogTemp, Warning, TEXT("Input binding for 'Enter' key is set."));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to bind input for 'Enter' key."));
	}
	if (StartWithCutscene)
	{
		if (!WBP_MainMenu)
		{
			WBP_MainMenu = StaticLoadClass(UUserWidget::StaticClass(), nullptr, TEXT("/Game/Mahidol_Map/Mainmenu/WBP_MainMenu.WBP_MainMenu_C"));
		}
		if (WBP_MainMenu)
		{
			// Create the WBP_MainMenu widget instance
			WidgetInstance_MainMenu = CreateWidget<UUserWidget>(GetWorld(), WBP_MainMenu);
			if (WidgetInstance_MainMenu)
			{
				// Add WBP_MainMenu to the viewport
				WidgetInstance_MainMenu->AddToViewport(2);
				UE_LOG(LogTemp, Warning, TEXT("WBP_MainMenu successfully created and added to viewport!"));
			}
		}

		PlaySequence();
	}
}


void AGM_Mahidol::GameModeViewTarget()
{
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerController not found!"));
		return;
	}
	else 
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerController was found!"));
	}
	APawn* PlayerActor = nullptr;
	for (TActorIterator<APawn> It(GetWorld()); It; ++It)
	{
		if (It->GetName().Contains(TEXT("BP_Character_TTBoyB")))
		{
			PlayerActor = *It;
			break;
		}
	}

	if (PlayerActor)
	{
		// Blend the view target to the specified actor
		PlayerController->SetViewTargetWithBlend(PlayerActor);
		UE_LOG(LogTemp, Log, TEXT("View target successfully changed to BP_Character_TTBoyB"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BP_Character_TTBoyB not found in the world!"));
	}

}

void AGM_Mahidol::PlaySequence()
{
	// Load Level Sequence Asset
	if (!LevelSequence)
	{
		LevelSequence = LoadObject<ULevelSequence>(nullptr, TEXT("/Game/Mahidol_Map/LevelSequence/MainMenuSequence.MainMenuSequence"));
	}

	if (LevelSequence)
	{
		PlayerController->SetIgnoreMoveInput(true);
		PlayerController->SetIgnoreLookInput(true);
		WidgetInstance_MainUI->SetVisibility(ESlateVisibility::Collapsed);
		UE_LOG(LogTemp, Warning, TEXT("LevelSequence was found!"));
		ALevelSequenceActor* SequenceActor;
		FMovieSceneSequencePlaybackSettings PlaybackSettings;
		

		// Create Level Sequence Player
		SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(
			GetWorld(),
			LevelSequence,
			PlaybackSettings,
			SequenceActor
		);

		if (SequencePlayer)
		{
			SequencePlayer->PlayLooping(-1);

			// //Get the CineCameraActor in the sequence and set it as the view target
			TArray<AActor*> CineCameras;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACineCameraActor::StaticClass(), CineCameras);
			if (CineCameras.Num() > 0)
			{
				PlayerController->SetViewTargetWithBlend(CineCameras[0], 0.5f);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("LevelSequence not found!"));
	}
}

void AGM_Mahidol::OnSequenceFinished()
{
	//PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController)
	{
		PlayerController->SetIgnoreMoveInput(false);
		PlayerController->SetIgnoreLookInput(false);
		WidgetInstance_MainUI->SetVisibility(ESlateVisibility::Visible);
		WidgetInstance_MainMenu->SetVisibility(ESlateVisibility::Hidden);
	}
}

void AGM_Mahidol::PlayStartSequence()
{
	if (bHasStartSequencePlayed)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayStartSequence() was already triggered. Ignoring subsequent input."));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("PlayStartSequence() called."));

	if (PlayerController && SequencePlayer && StartLevelSequence)
	{
		if (SequencePlayer->IsPlaying())
		{
			SequencePlayer->Stop();
			UE_LOG(LogTemp, Warning, TEXT("Stopped the current looping sequence."));
		}

		ALevelSequenceActor* StartSequenceActor;
		FMovieSceneSequencePlaybackSettings StartPlaybackSettings;
		StartPlaybackSettings.bDisableMovementInput = true;
		StartPlaybackSettings.bHideHud = true;

		ULevelSequencePlayer* StartSequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(
			GetWorld(),
			StartLevelSequence,
			StartPlaybackSettings,
			StartSequenceActor
		);
		if (PlayStartLevelSequence)
		{
			if (StartSequencePlayer)
			{
				StartSequencePlayer->OnFinished.AddDynamic(this, &AGM_Mahidol::OnSequenceFinished);
				StartSequencePlayer->Play();
				UE_LOG(LogTemp, Warning, TEXT("StartLevelSequence is playing."));
				bHasStartSequencePlayed = true;

				TArray<AActor*> CineCameras;
				UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACineCameraActor::StaticClass(), CineCameras);
				if (CineCameras.Num() > 0)
				{
					PlayerController->SetViewTargetWithBlend(CineCameras[0]);
				}
			}
			bHasStartSequencePlayed = true;
		}
		else
		{
			OnSequenceFinished();
			bHasStartSequencePlayed = true;
		}
		
	}
}

