// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Myrrha_DevGameModeBase.h"
#include "Containers/UnrealString.h"
#include "MenuGameMode.generated.h"


/**
 * 
 */
UCLASS()
class MYRRHA_DEV_API AMenuGameMode : public AMyrrha_DevGameModeBase
{
	GENERATED_BODY()
public:
	AMenuGameMode();
	virtual ~AMenuGameMode() = default;
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	TMap<float, FString>& GetLeaderboard();
private:
};
