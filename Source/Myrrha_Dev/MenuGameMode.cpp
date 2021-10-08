// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuGameMode.h"
#include "Misc/FileHelper.h"
#include "Kismet/GameplayStatics.h"
#include "SaveGameCpp.h"

AMenuGameMode::AMenuGameMode()
	:AMyrrha_DevGameModeBase()
{
}

void AMenuGameMode::BeginPlay()
{
	Super::BeginPlay();
}


TMap<float, FString>& AMenuGameMode::GetLeaderboard()
{
	USaveGameCpp* save;
	if (UGameplayStatics::DoesSaveGameExist("save", 0))
	{
		save = Cast<USaveGameCpp>(UGameplayStatics::LoadGameFromSlot("save", 0));
	}
	else
	{
		save = Cast<USaveGameCpp>(UGameplayStatics::CreateSaveGameObject(USaveGameCpp::StaticClass()));
	}
	return save->m_LeaderBoardMap;
}