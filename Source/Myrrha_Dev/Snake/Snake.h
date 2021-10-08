// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Snake.generated.h"

class ASnakeSpawner;
class ALilypad;
class APoisonProjectile;
class USoundCue;
class UAudioComponent;
class UNiagaraComponent;

UCLASS()
class MYRRHA_DEV_API ASnake : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASnake();
	
	void SetSpawner(ASnakeSpawner* spawner);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	FVector m_Destination;

	UPROPERTY(EditAnywhere)
	float m_Speed;

	UStaticMeshComponent* m_pMesh;

	ASnakeSpawner* m_pOwner;

	UPROPERTY(VisibleAnywhere)
	ALilypad* m_pTarget;

	UPROPERTY(VisibleAnywhere)
	APoisonProjectile* m_pProjectile;

	UPROPERTY(VisibleAnywhere)
		float m_LastDistanceSquared;

	bool m_ProjectileShot;

	UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex,
			bool bFromSweep,
			const FHitResult& SweepResult);
	
	bool m_IsFleeing;
	float m_FleeRadius;
	float m_MinTargettingDistance;

	UAudioComponent* m_pSpitAudioComponent;
	USoundCue* m_pSpitAudioCue;

	UNiagaraComponent* m_pMovingParticleSystem;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void SetDestination(const FVector& destination);

};
