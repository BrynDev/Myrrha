// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SaveGameCpp.generated.h"

/**
 * 
 */
UCLASS()
class MYRRHA_DEV_API USaveGameCpp : public USaveGame
{
	GENERATED_BODY()
	
public:
	USaveGameCpp();
	UPROPERTY()
	TMap<float, FString> m_LeaderBoardMap;
};
