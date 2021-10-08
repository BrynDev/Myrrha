// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/HUD.h"
#include "Camera/CameraShake.h"
#include "MyrrhaCharacter.generated.h"


class AFruit;
class UWeightComponent;
class USoundCue;
class UAudioComponent;

UCLASS()
class MYRRHA_DEV_API AMyrrhaCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyrrhaCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintPure, Category="Lives")
	int GetNrLives();
	UFUNCTION(BlueprintPure, Category = "Throw")
	bool CanThrowFruit();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Menu")
	TSubclassOf<UUserWidget> m_PauseMenuClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Menu")
	TSubclassOf<UUserWidget> m_GameOverMenuClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CameraShake")
	TSubclassOf<UMatineeCameraShake> m_CameraShakeClass;

	UFUNCTION(BlueprintImplementableEvent, Category = "Damage")
	void ShowDamageOverlay();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:

	virtual void Jump() override;

	//linked to input
	void MoveRight(float value);
	void MoveForward(float value);

	void HandleGrabInput();
	void EnableCanThrow();
	void AttemptGrab();
	void HandleThrowInput();
	void HandleLobInput();
	void ThrowCarriedItem(const FVector throwVect);
	void HandleFallingInWater();
	void BounceUpFromWater();
	void OpenMenu(UUserWidget* pMenuToOpen);
	void OpenPauseMenu();

	UPROPERTY()
	UUserWidget* m_pPauseMenuWidget;
	UPROPERTY()
	UUserWidget* m_pGameOverWidget;

	UPROPERTY()
	AFruit* m_pCurrentlyGrabbedFruit;
	UPROPERTY()
	float m_CarryHeightOffset;
	UPROPERTY()
	float m_ThrowSpeed;
	UPROPERTY()
	float m_LobHeight;
	UPROPERTY()
	float m_LobForwardSpeed;	
	bool m_CanThrow;

	UPROPERTY(EditAnywhere)
	int m_Weight;
	UWeightComponent* m_pWeightComp;

	UPROPERTY(EditAnywhere)
	float m_KillZ;
	UPROPERTY()
	int m_NrLives;
	UPROPERTY()
	bool m_CanBounce;
	UPROPERTY()
	float m_AccumBounceResetTime;
	UPROPERTY()
	float m_BounceResetTime;

	UAudioComponent* m_pJumpAudioComponent;
	USoundCue* m_pJumpAudioCue;
	UAudioComponent* m_pGrabAudioComponent;
	USoundCue* m_pGrabAudioCue;
	UAudioComponent* m_pThrowAudioComponent;
	USoundCue* m_pThrowAudioCue;
	UAudioComponent* m_pWaterAudioComponent;
	USoundCue* m_pWaterAudioCue;
	UAudioComponent* m_pFailedGrabAudioComponent;
	USoundCue* m_pFailedGrabAudioCue;


};
