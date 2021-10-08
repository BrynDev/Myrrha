// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FruitSpawner.generated.h"

class AFruit;
class USoundCue;
class UAudioComponent;

UCLASS()
class MYRRHA_DEV_API AFruitSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFruitSpawner();
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void DeactivateFruit(AFruit* actor);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	float m_FruitSpawnCounter;

	UPROPERTY(EditAnywhere)
	float m_FruitSpawnDelay;
	UPROPERTY(EditAnywhere)
	float m_MinFruitSpawnDelay;
	UPROPERTY(EditAnywhere)
	int m_SecondsBetweenDelayDecrease;
	UPROPERTY(EditAnywhere)
	int m_StartBufferSeconds; //time until delay starts decreasing
	UPROPERTY(EditAnywhere)
	float m_DelayDecreaseAmount;

	UPROPERTY(EditAnywhere)
	float m_FruitSpawnHeight;

	UPROPERTY(EditAnywhere)
	int m_FruitPoolSize;

	int m_ActiveFruits;

	TSubclassOf<class AFruit> m_FruitBlueprint;
	void SpawnFruit(const FVector& location);
	void UpdateSpawnDelay();

	UPROPERTY(VisibleAnywhere)
	TArray<AActor*> m_FruitArray;

	UStaticMesh* m_SMFruit_01;
	UStaticMesh* m_SMFruit_02;
	UStaticMesh* m_SMFruit_03;

	UAudioComponent* m_pSpawnAudioComp;
	USoundCue* m_pSpawnAudioCue;


};
