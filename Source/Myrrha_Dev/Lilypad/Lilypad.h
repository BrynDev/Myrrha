// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Lilypad.generated.h"

class USoundCue;
class UAudioComponent;
class UNiagaraComponent;

UCLASS()
class MYRRHA_DEV_API ALilypad : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALilypad();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Dissolve();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void LoadNearSinkParticle();

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* m_pMesh;

	UPROPERTY(EditAnywhere)
	float m_MaxZHeight;

	UPROPERTY(EditAnywhere)
	float m_MinZHeight;

	UPROPERTY(EditAnywhere)
	float m_NearSinkZHeight;
	UPROPERTY()
	UNiagaraComponent* m_pNearSinkParticle; //particle that plays when the fruit is nearly sunk
	FVector m_StartPos;

	UPROPERTY(EditAnywhere)
	float m_UpwardsForce;

	UPROPERTY(EditAnywhere)
	float m_SinkMultiplier;

	UPROPERTY(EditAnywhere)
	float m_FloatDelay;
	float m_FloatTimer;
	bool m_FloatingUp;
	bool m_IsNearSink;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* m_pTriggerVolume;

	UPROPERTY(VisibleAnywhere)
	int m_WeightCarried;

	//Dissolve effect
	TSubclassOf<class AActor> m_DissolveBP;
	AActor* m_pDissolveObject;
	UPROPERTY(EditAnywhere)
	float m_DissolveDuration;
	float m_DissolveCounter; 
	bool m_IsDissolving;

	// Overlap
	UFUNCTION()
	void BeginOverlap(UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex,
			bool bFromSweep,
			const FHitResult& SweepResult);

	UFUNCTION()
	void EndOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UAudioComponent* m_pDissolveAudioComponent;
	USoundCue* m_pDissolveAudioCue;

	UAudioComponent* m_pSinkAudioComponent;
	USoundCue* m_pSinkAudioCue;

	UAudioComponent* m_pWarningAudioComponent;
	USoundCue* m_pWarningAudioCue;



	bool m_ToBeDeleted;
	float m_DeleteCounter;
};
