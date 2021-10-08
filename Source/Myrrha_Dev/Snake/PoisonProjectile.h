// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "PoisonProjectile.generated.h"



class UCapsuleComponent;
class UNiagaraComponent;

UCLASS()
class MYRRHA_DEV_API APoisonProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APoisonProjectile();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void ShootAtTarget(AActor* Target, const FVector& startPos);

	bool IsActive();

	void Neutralized();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
private:
	UPROPERTY(EditAnywhere)
	float m_Speed;

	bool m_Active;

	AActor* m_pTarget;

	UCapsuleComponent* m_pCollision;

	UNiagaraComponent* m_pTravelingParticleSystem; //particle that follows the fruit and plays while it is grabbed

	TSubclassOf<class AActor> m_DissolveBP;
	AActor* m_pDissolveObject;
	
	FVector m_Velocity;

};
