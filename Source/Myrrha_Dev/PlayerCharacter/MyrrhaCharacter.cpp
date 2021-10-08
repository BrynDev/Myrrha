// Fill out your copyright notice in the Description page of Project Settings.


#include "MyrrhaCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../Fruit/Fruit.h"
#include "../WeightComponent/WeightComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"


// Sets default values
AMyrrhaCharacter::AMyrrhaCharacter()
	: m_pCurrentlyGrabbedFruit{nullptr}
	, m_CarryHeightOffset{200.f}
	, m_ThrowSpeed{ 1800.f }	
	, m_LobHeight{700.f}
	, m_LobForwardSpeed{70.f}
	, m_Weight{10}
	, m_pWeightComp{ nullptr }
	, m_KillZ{-2.f}
	, m_NrLives{2}
	, m_CanBounce{true}
	, m_AccumBounceResetTime{0.f}
	, m_BounceResetTime{0.2f}
	, m_CanThrow{false}
{
	PrimaryActorTick.bCanEverTick = true;
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	UCharacterMovementComponent* pCharMovement{ GetCharacterMovement() };
	pCharMovement->bOrientRotationToMovement = true;
	pCharMovement->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // Character turns in the direction of the movement
	pCharMovement->JumpZVelocity = 400.f;
	pCharMovement->AirControl = 0.8f;
	pCharMovement->bConstrainToPlane = true;
	pCharMovement->bSnapToPlaneAtStart = true;

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	USkeletalMeshComponent* pMesh{ GetMesh() };
	pMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	pMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	pMesh->SetNotifyRigidBodyCollision(false); //generate hit events set to false
	pMesh->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);

	//weight component
	m_pWeightComp = CreateDefaultSubobject<UWeightComponent>(TEXT("Weight"));

	//sounds
	m_pJumpAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("jumpAudioComp"));
	m_pJumpAudioComponent->bAutoActivate = false;
	m_pJumpAudioComponent->SetupAttachment(RootComponent);

	ConstructorHelpers::FObjectFinder<USoundCue> soundAsset(TEXT("SoundCue'/Game/Audio/SFX/S_Player_Jump_01_Cue.S_Player_Jump_01_Cue'"));
	if (soundAsset.Succeeded())
	{
		m_pJumpAudioCue = soundAsset.Object;
	}
	if (m_pJumpAudioComponent->IsValidLowLevelFast())
	{
		m_pJumpAudioComponent->SetSound(m_pJumpAudioCue);
	}

	m_pGrabAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("grabAudioComp"));
	m_pGrabAudioComponent->bAutoActivate = false;
	m_pGrabAudioComponent->SetupAttachment(RootComponent);

	ConstructorHelpers::FObjectFinder<USoundCue> soundAsset2(TEXT("SoundCue'/Game/Audio/SFX/S_Fruit_Grab_Cue.S_Fruit_Grab_Cue'"));
	if (soundAsset2.Succeeded())
	{
		m_pGrabAudioCue = soundAsset2.Object;
	}
	if (m_pGrabAudioComponent->IsValidLowLevelFast())
	{
		m_pGrabAudioComponent->SetSound(m_pGrabAudioCue);
	}

	m_pThrowAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("throwAudioComp"));
	m_pThrowAudioComponent->bAutoActivate = false;
	m_pThrowAudioComponent->SetupAttachment(RootComponent);

	ConstructorHelpers::FObjectFinder<USoundCue> soundAsset3(TEXT("SoundCue'/Game/Audio/SFX/S_Fruit_Throw_Cue.S_Fruit_Throw_Cue'"));
	if (soundAsset3.Succeeded())
	{
		m_pThrowAudioCue = soundAsset3.Object;
	}
	if (m_pThrowAudioComponent->IsValidLowLevelFast())
	{
		m_pThrowAudioComponent->SetSound(m_pThrowAudioCue);
	}

	m_pWaterAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("waterAudioComp"));
	m_pWaterAudioComponent->bAutoActivate = false;
	m_pWaterAudioComponent->SetupAttachment(RootComponent);

	ConstructorHelpers::FObjectFinder<USoundCue> soundAsset4(TEXT("SoundCue'/Game/Audio/SFX/S_Player_WaterJump_Cue.S_Player_WaterJump_Cue'"));
	if (soundAsset4.Succeeded())
	{
		m_pWaterAudioCue = soundAsset4.Object;
	}
	if (m_pWaterAudioComponent->IsValidLowLevelFast())
	{
		m_pWaterAudioComponent->SetSound(m_pWaterAudioCue);
	}

	m_pFailedGrabAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("failedGrabAudioComp"));
	m_pFailedGrabAudioComponent->bAutoActivate = false;
	m_pFailedGrabAudioComponent->SetupAttachment(RootComponent);

	ConstructorHelpers::FObjectFinder<USoundCue> soundAsset5(TEXT("SoundCue'/Game/Audio/SFX/S_GrabAttempt_Cue.S_GrabAttempt_Cue'"));
	if (soundAsset5.Succeeded())
	{
		m_pFailedGrabAudioCue = soundAsset5.Object;
	}
	if (m_pFailedGrabAudioComponent->IsValidLowLevelFast())
	{
		m_pFailedGrabAudioComponent->SetSound(m_pFailedGrabAudioCue);
	}


}

// Called when the game starts or when spawned
void AMyrrhaCharacter::BeginPlay()
{
	Super::BeginPlay();
	JumpMaxHoldTime = .35f; //setting in constructor doesnt work

	m_pWeightComp->SetWeight(m_Weight);
	m_pWeightComp->RegisterComponent();

	if (m_PauseMenuClass)
	{
		m_pPauseMenuWidget = CreateWidget<UUserWidget>(GetWorld(), m_PauseMenuClass);
		m_pPauseMenuWidget->AddToViewport();
		m_pPauseMenuWidget->bIsFocusable = true;
		m_pPauseMenuWidget->SetVisibility(ESlateVisibility::Hidden);
		m_pPauseMenuWidget->SetIsEnabled(false);
	}

	if (m_GameOverMenuClass)
	{
		m_pGameOverWidget = CreateWidget<UUserWidget>(GetWorld(), m_GameOverMenuClass);
		m_pGameOverWidget->AddToViewport();
		m_pGameOverWidget->bIsFocusable = true;
		m_pGameOverWidget->SetVisibility(ESlateVisibility::Hidden);
		m_pGameOverWidget->SetIsEnabled(false);
	}
	
	//camera fade in, needs to be done from the player to access the controller
	APlayerController* pPlayerController = Cast<APlayerController>(Controller);
	pPlayerController->ClientSetCameraFade(true, FColor::Black, FVector2D(1.0, 0.0), 1.5f);
}

// Called every frame
void AMyrrhaCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (m_pCurrentlyGrabbedFruit != nullptr)
	{
		FVector fruitLocation{ GetActorLocation() };
		fruitLocation.Z += m_CarryHeightOffset;

		//interpolated movement
		FLatentActionInfo latentInfo{};
		latentInfo.CallbackTarget = this;
		float lerpSpeed{ 0.10f };
		//if the player is jumping, decrease lerp speed so the fruit more tightly follows the player
		//this prevents the player from bumping their head against the fruit and losing their upwards velocity
		if (GetVelocity().Z > 0.f || !m_CanBounce)
		{
			lerpSpeed = 0.f;
		}
		UKismetSystemLibrary::MoveComponentTo(m_pCurrentlyGrabbedFruit->GetRootComponent(), fruitLocation, GetActorRotation(), true, false, lerpSpeed, true, EMoveComponentAction::Type::Move, latentInfo);
	}

	if (!m_CanBounce)
	{
		m_AccumBounceResetTime += DeltaTime;
		if (m_AccumBounceResetTime >= m_BounceResetTime)
		{
			m_AccumBounceResetTime = 0;
			m_CanBounce = true;
			GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}
	}

	if (GetActorLocation().Z < m_KillZ)
	{
		HandleFallingInWater();
	}
}

// Called to bind functionality to input
void AMyrrhaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMyrrhaCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMyrrhaCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMyrrhaCharacter::MoveRight);

	//PlayerInputComponent->BindAction("Grab", IE_Pressed, this, &AMyrrhaCharacter::AttemptGrab);
	PlayerInputComponent->BindAction("Grab", IE_Pressed, this, &AMyrrhaCharacter::HandleGrabInput);
	PlayerInputComponent->BindAction("Lob", IE_Released, this, &AMyrrhaCharacter::HandleLobInput);
	PlayerInputComponent->BindAction("Throw", IE_Released, this, &AMyrrhaCharacter::HandleThrowInput);
	PlayerInputComponent->BindAction("Grab", IE_Released, this, &AMyrrhaCharacter::EnableCanThrow);
	
	
	PlayerInputComponent->BindAction("Pause", IE_Pressed, this, &AMyrrhaCharacter::OpenPauseMenu);

}

void AMyrrhaCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		//The movement direction is a literal because this prevents a bug where the player controller gets rotated, resulting in the controls getting messed up.
		AddMovementInput(FVector(1,0,0), Value);
	}
}

void AMyrrhaCharacter::Jump()
{
	bPressedJump = true;
	JumpKeyHoldTime = 0.0f;
	if (CanJump())
	{
		m_pJumpAudioComponent->Play();
	}
}

void AMyrrhaCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{		
		//The movement direction is a literal because this prevents a bug where the player controller gets rotated, resulting in the controls getting messed up.
		AddMovementInput(FVector(0, 1, 0), Value);
	}
}

void AMyrrhaCharacter::HandleGrabInput()
{
	if (m_pCurrentlyGrabbedFruit == nullptr)
	{
		AttemptGrab();
	}
}

void AMyrrhaCharacter::EnableCanThrow()
{
	if (m_pCurrentlyGrabbedFruit !=nullptr)
	{
		m_CanThrow = true;
	}
}

void AMyrrhaCharacter::HandleThrowInput()
{
	if (m_pCurrentlyGrabbedFruit == nullptr || !m_CanThrow)
	{
		return;
	}

	FVector throwVect{ GetActorForwardVector() }; //already normalized
	throwVect *= m_ThrowSpeed;

	ThrowCarriedItem(throwVect);
}

void AMyrrhaCharacter::HandleLobInput()
{
	if (m_pCurrentlyGrabbedFruit == nullptr || !m_CanThrow)
	{
		return;
	}

	FVector throwVect{ GetActorForwardVector() }; //already normalized
	throwVect *= m_LobForwardSpeed;
	throwVect.Z = m_LobHeight;


	ThrowCarriedItem(throwVect);
}

void AMyrrhaCharacter::AttemptGrab()
{
	if (m_pCurrentlyGrabbedFruit != nullptr)
	{
		return;
	}
	FHitResult hitResult{};
	const FVector collisionBoxExtents{ 50.f, 50.f,50.f };
	const FCollisionShape collisionShape{FCollisionShape::MakeBox(collisionBoxExtents/2.f)};
	const FVector startPos{ GetActorLocation() };
	const FQuat rotation{ GetActorRotation() };

	FVector forwardVect{ GetActorForwardVector() };
	forwardVect.Normalize();
	const float grabRange{ 150.f };
	forwardVect = forwardVect * grabRange;
	const FVector endPos{ startPos + forwardVect };

	FCollisionObjectQueryParams params{ ECollisionChannel::ECC_WorldDynamic };
	
	if (GetWorld()->SweepSingleByObjectType(hitResult, startPos, endPos, rotation, params, collisionShape))
	{
		if (hitResult.Actor->IsA<AFruit>())
		{
			m_pGrabAudioComponent->Play();
			m_pCurrentlyGrabbedFruit = Cast<AFruit>(hitResult.Actor);
			m_pCurrentlyGrabbedFruit->HandleGrab();
		}
	}

	if (m_pCurrentlyGrabbedFruit == nullptr)
	{
		m_pFailedGrabAudioComponent->Play();
	}

	//play particle
	UNiagaraSystem* NS = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/RFX/Systems/PS_GrabEffect.PS_GrabEffect"), nullptr, LOAD_None, nullptr);
	if (NS->IsValid())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), NS, endPos, GetActorRotation());
	}
}

void AMyrrhaCharacter::ThrowCarriedItem(const FVector throwVect)
{
	//stop the interpolated movement first
	//not doing this causes the fruit to hitch for a moment before getting thrown
	FLatentActionInfo latentInfo{};
	latentInfo.CallbackTarget = this;
	UKismetSystemLibrary::MoveComponentTo(m_pCurrentlyGrabbedFruit->GetRootComponent(), m_pCurrentlyGrabbedFruit->GetActorLocation(), GetActorRotation(), false, false, 0.01f, true, EMoveComponentAction::Type::Stop, latentInfo);

	m_pThrowAudioComponent->Play();
	m_pCurrentlyGrabbedFruit->HandleThrow(throwVect);
	m_pCurrentlyGrabbedFruit = nullptr;
	m_CanThrow = false;
}

void AMyrrhaCharacter::HandleFallingInWater()
{
	if (m_NrLives > 0)
	{
		if (m_CanBounce)
		{
			--m_NrLives;
			BounceUpFromWater();
			m_CanBounce = false;
			GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			//play sound
			m_pWaterAudioComponent->Play();
			//play particle
			UNiagaraSystem* pNiagaraSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/RFX/Systems/PS_WaterSplashBig.PS_WaterSplashBig"), nullptr, LOAD_None, nullptr);
			if (pNiagaraSystem->IsValid())
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), pNiagaraSystem, GetActorLocation());
			}

			//show overlay (blueprint implementation)
			ShowDamageOverlay();

			//play screenshake
			if (m_CameraShakeClass != NULL)
			{
				//GetWorld()->GetFirstPlayerController()->PlayerCameraManager->PlayCameraShake(m_CameraShakeClass);
				GetWorld()->GetFirstPlayerController()->PlayerCameraManager->StartCameraShake(m_CameraShakeClass);
			}
		}
	}
	else
	{

		//end the game
		OpenMenu(m_pGameOverWidget);
	}
}

void AMyrrhaCharacter::BounceUpFromWater()
{
	const float launchVel{ 1300.f };
	LaunchCharacter(FVector(0, 0, launchVel), false, true);
}

void AMyrrhaCharacter::OpenMenu(UUserWidget* pMenuToOpen)
{
	pMenuToOpen->SetVisibility(ESlateVisibility::Visible);
	pMenuToOpen->SetIsEnabled(true);
	APlayerController* pController{ UGameplayStatics::GetPlayerController(GetWorld(), 0) };
	FInputModeUIOnly inputMode{};
	inputMode.SetWidgetToFocus(pMenuToOpen->GetCachedWidget());
	pController->SetInputMode(inputMode);
	pController->SetPause(true);
	pMenuToOpen->SetFocus();
	pMenuToOpen->SetUserFocus(pController);
}

void AMyrrhaCharacter::OpenPauseMenu()
{
	OpenMenu(m_pPauseMenuWidget);
}

int AMyrrhaCharacter::GetNrLives()
{
	return m_NrLives;
}

bool AMyrrhaCharacter::CanThrowFruit()
{
	return m_CanThrow;
}

