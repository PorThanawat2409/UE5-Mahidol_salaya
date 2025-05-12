// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "APIService.h"
#include "MahidolPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "LevelSequence.h"
#include "LevelSequencePlayer.h"
#include "LevelSequenceActor.h"
#include "CineCameraActor.h"
#include "GameFramework/GameModeBase.h"
#include "GM_Mahidol.generated.h"

UCLASS()
class MYPROJECT_API AGM_Mahidol : public AGameModeBase
{
	GENERATED_BODY()

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	bool bHasStartSequencePlayed = false;

public:
	AGM_Mahidol();


	UPROPERTY(EditAnyWhere, BlueprintReadWrite) 
	AAPIService* APIService = nullptr;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	AMahidolPlayerController* PlayerController = nullptr;

	UFUNCTION(BlueprintCallable) 
	void GameModeViewTarget();

	UFUNCTION(BlueprintCallable)
	void PlaySequence();
	
	UFUNCTION(BlueprintCallable)
	void OnSequenceFinished();

	UFUNCTION(BlueprintCallable)
	void PlayStartSequence();
	
	UUserWidget* WB_hint_bar = nullptr;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Sequence")
	bool StartWithCutscene = false;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Sequence")
	bool PlayStartLevelSequence = false;
	//// Custom HUD class
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
	//TSubclassOf<class AHUD> HUDClass ;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sequence")
	ULevelSequence* LevelSequence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sequence")
	ULevelSequence* StartLevelSequence;

	ULevelSequencePlayer* SequencePlayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> WBP_MainUI;

	UUserWidget* WidgetInstance_MainUI;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> WBP_MainMenu;

	UUserWidget* WidgetInstance_MainMenu;
	

};
