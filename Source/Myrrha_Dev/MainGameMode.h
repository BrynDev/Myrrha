// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Containers/UnrealString.h"
#include "MainGameMode.generated.h"


class USaveGameCpp;
/**
 * 
 */
UCLASS()
class MYRRHA_DEV_API AMainGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	AMainGameMode();

	virtual ~AMainGameMode();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void AddScoreToLeaderBoard(const FString& name);

	UFUNCTION(BlueprintCallable)
	TMap<float, FString>& GetLeaderboard();

	UFUNCTION(BlueprintPure, Category = "Score")
	float GetScore();

	UFUNCTION(BlueprintPure, Category = "SurvivedTime")
	int GetSurvivedMinutes();
	UFUNCTION(BlueprintPure, Category = "SurvivedTime")
	int GetSurvivedSeconds();

	UFUNCTION(BlueprintImplementableEvent, Category = "NewHighScore")
	void DoNewHighScoreEffects();

	void AddScore(float score);
private:
	int m_SurvivedMinutes;
	int m_SurvivedSeconds;
	float m_ElapsedTime;
	float m_Score;

	void WriteLeaderBoard();


	USaveGameCpp* m_pSaveGame;
};
