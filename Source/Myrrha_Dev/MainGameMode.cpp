// Fill out your copyright notice in the Description page of Project Settings.


#include "MainGameMode.h"
#include "Misc/FileHelper.h"
#include "Kismet/GameplayStatics.h"
#include "SaveGameCpp.h"

AMainGameMode::AMainGameMode()
	:AGameModeBase()
	, m_SurvivedMinutes{0}
	, m_SurvivedSeconds{0}
	, m_ElapsedTime{0}
	, m_Score{0}

{
	PrimaryActorTick.bCanEverTick = true;
	ConstructorHelpers::FObjectFinder<USoundWave> soundAsset(TEXT("SoundWave'/Game/Audio/Music/S_Background.S_Background'"));

	UGameplayStatics::PlaySound2D(GetWorld(), soundAsset.Object, 0.2);
	if (!UGameplayStatics::DoesSaveGameExist("save", 0))
	{
		m_pSaveGame = Cast<USaveGameCpp>(UGameplayStatics::CreateSaveGameObject(USaveGameCpp::StaticClass()));

		UGameplayStatics::SaveGameToSlot(m_pSaveGame, "save", 0);
	}

	m_pSaveGame = Cast<USaveGameCpp>(UGameplayStatics::LoadGameFromSlot("save", 0));
}

AMainGameMode::~AMainGameMode()
{
}

void AMainGameMode::BeginPlay()
{
	Super::BeginPlay();
	if (m_pSaveGame == nullptr || !m_pSaveGame->IsValidLowLevel())
	{
		m_pSaveGame = Cast<USaveGameCpp>(UGameplayStatics::LoadGameFromSlot("save", 0));
	}
	m_pSaveGame->m_LeaderBoardMap.KeyStableSort([](float A, float B) { return A > B; });
}

void AMainGameMode::Tick(float DeltaTime)
{
	m_ElapsedTime += DeltaTime;

	if (m_ElapsedTime >= 1.f)
	{
		--m_ElapsedTime;
		++m_SurvivedSeconds;
		if (m_SurvivedSeconds == 60)
		{
			m_SurvivedSeconds = 0;
			++m_SurvivedMinutes;
		}
	}	
}

void AMainGameMode::AddScoreToLeaderBoard(const FString& name)
{
	if (m_pSaveGame == nullptr || !m_pSaveGame->IsValidLowLevel())
	{
		m_pSaveGame = Cast<USaveGameCpp>(UGameplayStatics::LoadGameFromSlot("save", 0));
	}
	float score{ GetScore() };
	m_pSaveGame->m_LeaderBoardMap.Add(score, name);
	m_pSaveGame->m_LeaderBoardMap.KeyStableSort([](float A, float B) { return A > B; });
	WriteLeaderBoard();

	//check if this score is the new high score
	if (FMath::IsNearlyEqual(m_pSaveGame->m_LeaderBoardMap.begin().Key(), score))
	{
		//blueprint implementation
		DoNewHighScoreEffects();
	}
	
}

TMap<float, FString>& AMainGameMode::GetLeaderboard()
{
	if (m_pSaveGame == nullptr || !m_pSaveGame->IsValidLowLevel())
	{
		m_pSaveGame = Cast<USaveGameCpp>(UGameplayStatics::LoadGameFromSlot("save", 0));
		return m_pSaveGame->m_LeaderBoardMap;
	}
	return m_pSaveGame->m_LeaderBoardMap;
}

float AMainGameMode::GetScore()
{
	return m_SurvivedMinutes * 60 + m_SurvivedSeconds + m_ElapsedTime + m_Score;
}

int AMainGameMode::GetSurvivedMinutes()
{
	return m_SurvivedMinutes;
}

int AMainGameMode::GetSurvivedSeconds()
{
	return m_SurvivedSeconds;
}

void AMainGameMode::AddScore(float score)
{
	m_Score += score;
}

void AMainGameMode::WriteLeaderBoard()
{
	if (m_pSaveGame == nullptr || !m_pSaveGame->IsValidLowLevel())
	{
		m_pSaveGame = Cast<USaveGameCpp>(UGameplayStatics::LoadGameFromSlot("save", 0));
	}
	UGameplayStatics::SaveGameToSlot(m_pSaveGame, "save", 0);
}

