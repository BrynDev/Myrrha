// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Fruit.generated.h"

class UCapsuleComponent;
class UWeightComponent;
class AFruitSpawner;
class UNiagaraComponent;
class USoundCue;
class UAudioComponent;

UCLASS()
class MYRRHA_DEV_API AFruit : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFruit();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SetSpawner(AFruitSpawner* spawner);

	void HandleGrab();
	void HandleThrow(const FVector& throwVector);
	void SetStaticMesh(UStaticMesh* pStaticMesh);
	void SetRandomAngularVelocity();

	UFUNCTION(BlueprintImplementableEvent, Category = "Fruit")
	void OnGiveScoreWater();
	UFUNCTION(BlueprintImplementableEvent, Category = "Fruit")
	void OnGiveScorePoison();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:

	

	UPROPERTY(EditAnywhere)
	int m_Weight;
	
	UPROPERTY(Category = Mesh, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* m_pStaticMesh;

	UPROPERTY(Category = Collider, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent* m_pCollision;

	UWeightComponent* m_pWeightComp;

	UNiagaraComponent* m_pCarriedParticleSystem; //particle that follows the fruit and plays while it is grabbed

	AFruitSpawner* m_pSpawner;

	UPROPERTY(EditAnywhere)
	float m_WaterLevel;

	UPROPERTY(EditAnywhere)
	float m_DespawnDistance;

	//gravity is -1500
	UPROPERTY(EditAnywhere)
	float m_DefaultUpAccel;
	UPROPERTY(EditAnywhere)
	float m_ThrownUpAccel;
	float m_CurrentUpAccel;

	bool m_WasThrown;
	bool m_WasHit;

	float m_PointsWater;

	float m_PointsPoison;

	void Deactivate();

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex,
			bool bFromSweep,
			const FHitResult& SweepResult);
	
	UAudioComponent* m_pWaterAudioComponent;
	USoundCue* m_pWaterAudioCue;

	UAudioComponent* m_pCollisionAudioComponent;
	USoundCue* m_pCollisionAudioCue;
};
