// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SnakeSpawner.generated.h"

class ASnake;

UCLASS()
class MYRRHA_DEV_API ASnakeSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASnakeSpawner();

	void DeactivateSnake(ASnake* snake);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	float m_SpawnCounter;

	UPROPERTY(EditAnywhere)
	float m_SpawnDelay;
	UPROPERTY(EditAnywhere)
	float m_MinSpawnDelay;
	UPROPERTY(EditAnywhere)
	int m_SecondsBetweenDelayDecrease;
	UPROPERTY(EditAnywhere)
	int m_StartBufferSeconds; //time until delay starts decreasing
	UPROPERTY(EditAnywhere)
	float m_DelayDecreaseAmount;

	UPROPERTY(EditAnywhere)
	float m_SnakeHeight;

	//distance from center where snakes are spawned
	UPROPERTY(EditAnywhere)
	float m_Radius;

	UPROPERTY(EditAnywhere)
	int m_PoolSize;

	UPROPERTY(VisibleAnywhere)
	int m_ActiveSnakes;

	UPROPERTY(VisibleAnywhere)
	TArray<AActor*> m_SnakeArray;

	void SpawnSnake(const FVector& location, const FVector& destination);
	void UpdateSpawnDelay();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
