// Fill out your copyright notice in the Description page of Project Settings.


#include "SnakeSpawner.h"
#include "Snake.h"
#include "Math/UnrealMathUtility.h"
#include "../MainGameMode.h"

// Sets default values
ASnakeSpawner::ASnakeSpawner()
	:m_SpawnCounter {0.f}
	, m_SpawnDelay{ 15.f }
	, m_MinSpawnDelay{ 8.f }
	, m_SecondsBetweenDelayDecrease{ 90 }
	, m_StartBufferSeconds{ 60 }
	, m_DelayDecreaseAmount{ 1.5f }
	, m_SnakeHeight{ 900.f }
	, m_Radius{ 2000.f }
	, m_PoolSize{ 3 }
	, m_ActiveSnakes{ 0 }
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ASnakeSpawner::DeactivateSnake(ASnake* snake)
{
	for (int i = 0; i < m_ActiveSnakes; i++)
	{
		if (m_SnakeArray[i] == snake)
		{
			snake->SetActorHiddenInGame(true);
			snake->SetActorEnableCollision(false);
			snake->SetActorTickEnabled(false);
			AActor* temp = m_SnakeArray[m_ActiveSnakes - 1];
			m_SnakeArray[m_ActiveSnakes - 1] = snake;
			m_SnakeArray[i] = temp;
			--m_ActiveSnakes;
		}
	}
}

// Called when the game starts or when spawned
void ASnakeSpawner::BeginPlay()
{
	Super::BeginPlay();
	
	auto world = GetWorld();
	m_SnakeArray.Reserve(m_PoolSize);
	for (int i = 0; i < m_PoolSize; i++)
	{
		auto snake = world->SpawnActor(ASnake::StaticClass());
		snake->SetActorHiddenInGame(true);
		snake->SetActorEnableCollision(false);
		snake->SetActorTickEnabled(false);
		((ASnake*)snake)->SetSpawner(this);
		m_SnakeArray.Push(snake);
	}
}

void ASnakeSpawner::SpawnSnake(const FVector& location, const FVector& destination)
{
	if (m_ActiveSnakes == m_PoolSize)
	{
		UE_LOG(LogTemp, Warning, TEXT("Increase ObjectPool size"))
			auto world = GetWorld();
		m_PoolSize *= 2;
		m_SnakeArray.Reserve(m_PoolSize); //when array runs out of space double the capacity, shouldnt happen but this is a safety measure just in case
		for (int i = 0; i < m_PoolSize / 2; i++)
		{
			auto snake = world->SpawnActor(ASnake::StaticClass());
			snake->SetActorHiddenInGame(true);
			snake->SetActorEnableCollision(false);
			snake->SetActorTickEnabled(false);
			((ASnake*)snake)->SetSpawner(this);
			m_SnakeArray.Push(snake);
		}
	}
	if (m_SnakeArray[m_ActiveSnakes] == nullptr || !m_SnakeArray[m_ActiveSnakes]->IsValidLowLevel())
	{
		m_SnakeArray[m_ActiveSnakes] = GetWorld()->SpawnActor(ASnake::StaticClass());
		m_SnakeArray[m_ActiveSnakes]->SetActorHiddenInGame(true);
		m_SnakeArray[m_ActiveSnakes]->SetActorEnableCollision(false);
		m_SnakeArray[m_ActiveSnakes]->SetActorTickEnabled(false);
		((ASnake*)m_SnakeArray[m_ActiveSnakes])->SetSpawner(this);
	}

	m_SnakeArray[m_ActiveSnakes]->SetActorLocation(location);
	m_SnakeArray[m_ActiveSnakes]->SetActorHiddenInGame(false);
	m_SnakeArray[m_ActiveSnakes]->SetActorEnableCollision(true);
	m_SnakeArray[m_ActiveSnakes]->SetActorTickEnabled(true);
	((ASnake*)m_SnakeArray[m_ActiveSnakes])->SetDestination(destination);
	++m_ActiveSnakes;
}

// Called every frame
void ASnakeSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	m_SpawnCounter += DeltaTime;
	if (m_SpawnCounter > m_SpawnDelay)
	{
		UE_LOG(LogTemp, Warning, TEXT("Spawning snake"))
		m_SpawnCounter -= m_SpawnDelay;
		float angle = FMath::FRand() * 360.f; //random angle
		FVector destination;
		FVector location;
		location.Z = destination.Z = m_SnakeHeight;
		location.X = cosf(FMath::DegreesToRadians(angle)) * m_Radius;
		location.Y = sinf(FMath::DegreesToRadians(angle)) * m_Radius;
		angle += 180.f;
		destination.X = cosf(FMath::DegreesToRadians(angle)) * m_Radius;
		destination.Y = sinf(FMath::DegreesToRadians(angle)) * m_Radius;
		SpawnSnake(location, destination);
		UpdateSpawnDelay();
	}

}

void ASnakeSpawner::UpdateSpawnDelay()
{
	AMainGameMode* pGameMode{ GetWorld()->GetAuthGameMode<AMainGameMode>() };
	int survivedMinutes{ pGameMode->GetSurvivedMinutes() };
	int survivedSeconds{ pGameMode->GetSurvivedSeconds() };
	int totalSeconds{ survivedSeconds + 60 * survivedMinutes };

	//if delay has reached minimum value or the delay cannot start decreasing yet, early exit
	if (m_SpawnDelay <= m_MinSpawnDelay || totalSeconds < m_StartBufferSeconds)
	{
		return;
	}

	if (totalSeconds % m_SecondsBetweenDelayDecrease == 0)
	{
		m_SpawnDelay -= m_DelayDecreaseAmount;

		//clamp the value if necessary
		if (m_SpawnDelay < m_SpawnDelay)
		{
			m_SpawnDelay = m_MinSpawnDelay;
		}
	}
}
