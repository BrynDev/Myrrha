// Fill out your copyright notice in the Description page of Project Settings.


#include "Lilypad.h"
#include "Components/CapsuleComponent.h"
#include "../WeightComponent/WeightComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"

// Sets default values
ALilypad::ALilypad()
	:  m_pMesh{nullptr}
	, m_MaxZHeight{ 50.f }
	, m_MinZHeight{ 0.f }
	, m_NearSinkZHeight{ 18.f }
	, m_pNearSinkParticle{nullptr}
	, m_UpwardsForce{ 20.f }
	, m_SinkMultiplier{ -1.f }
	, m_FloatDelay{ 1.f }
	, m_FloatTimer{ 0 }
	, m_FloatingUp{ true }
	, m_IsNearSink{false}
	, m_pTriggerVolume{nullptr}
	, m_WeightCarried{ 0 }
	, m_DissolveBP{NULL}
	, m_pDissolveObject{nullptr}
	, m_DissolveDuration{1.f}
	, m_DissolveCounter{ 0 }
	, m_IsDissolving{false}
	, m_ToBeDeleted{false}
	, m_DeleteCounter{0.f}
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	m_pMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	m_pMesh->SetupAttachment(RootComponent);

	ConstructorHelpers::FObjectFinder<UStaticMesh> lilypadAsset(TEXT("StaticMesh'/Game/Environment/Meshes/LilyPads/Lilypad_Platform_01.Lilypad_Platform_01'"));

	if (lilypadAsset.Succeeded())
	{
		m_pMesh->SetStaticMesh(lilypadAsset.Object);
		m_pMesh->SetRelativeScale3D(FVector{ 1.f, 1.f, 1.f });
	}
	
	m_pTriggerVolume = CreateDefaultSubobject<UStaticMeshComponent>(FName("TriggerVolume"));
	m_pTriggerVolume->SetupAttachment(m_pMesh);

	ConstructorHelpers::FObjectFinder<UStaticMesh> cylinder(TEXT("StaticMesh'/Engine/BasicShapes/Cylinder.Cylinder'"));
	if (cylinder.Succeeded())
	{
		m_pTriggerVolume->SetStaticMesh(cylinder.Object);
		m_pTriggerVolume->SetRelativeScale3D(FVector{ 3.6f, 3.6f, 0.15f });
		m_pTriggerVolume->SetRelativeLocation(FVector{ 0.f, 0.f , 30.f });
		m_pTriggerVolume->SetHiddenInGame(true);
		m_pTriggerVolume->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		m_pTriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &ALilypad::BeginOverlap);
		m_pTriggerVolume->OnComponentEndOverlap.AddDynamic(this, &ALilypad::EndOverlap);
	}

	m_pDissolveAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("dissolveAudioComp"));

	m_pDissolveAudioComponent->bAutoActivate = false;

	m_pDissolveAudioComponent->SetupAttachment(RootComponent);

	ConstructorHelpers::FObjectFinder<USoundCue> soundAsset(TEXT("'/Game/Audio/SFX/S_Snake_PoisonDissolve_Cue.S_Snake_PoisonDissolve_Cue'"));
	if (soundAsset.Succeeded())
	{
		m_pDissolveAudioCue = soundAsset.Object;
	}

	if (m_pDissolveAudioComponent->IsValidLowLevelFast())
	{
		m_pDissolveAudioComponent->SetSound(m_pDissolveAudioCue);
	}

	m_pSinkAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("sinkAudioComp"));

	m_pSinkAudioComponent->bAutoActivate = false;

	m_pSinkAudioComponent->SetupAttachment(RootComponent);

	ConstructorHelpers::FObjectFinder<USoundCue> soundAsset2(TEXT("SoundCue'/Game/Audio/SFX/S_LilyPadSunk_Cue.S_LilyPadSunk_Cue'"));
	if (soundAsset2.Succeeded())
	{
		m_pSinkAudioCue = soundAsset2.Object;
	}

	if (m_pSinkAudioComponent->IsValidLowLevelFast())
	{
		m_pSinkAudioComponent->SetSound(m_pSinkAudioCue);
	}

	m_pWarningAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("warningAudioComp"));

	m_pWarningAudioComponent->bAutoActivate = false;

	m_pWarningAudioComponent->SetupAttachment(RootComponent);

	ConstructorHelpers::FObjectFinder<USoundCue> soundAsset3(TEXT("SoundCue'/Game/Audio/SFX/S_LilyPadWarning_Cue.S_LilyPadWarning_Cue'"));
	if (soundAsset3.Succeeded())
	{
		m_pWarningAudioCue = soundAsset3.Object;
	}

	if (m_pWarningAudioComponent->IsValidLowLevelFast())
	{
		m_pWarningAudioComponent->SetSound(m_pWarningAudioCue);
	}

	//dissolve blueprint
	ConstructorHelpers::FObjectFinder<UClass> dissolveBP(TEXT("Class'/Game/BP_Dissolve.BP_Dissolve_C'"));
	if (dissolveBP.Object)
	{
		m_DissolveBP = dissolveBP.Object;
	}
}

void ALilypad::Dissolve()
{
	m_pDissolveAudioComponent->Play();
	m_IsDissolving = true;
	if (m_pNearSinkParticle != nullptr)
	{
		m_pNearSinkParticle->Deactivate();
	}

	if (m_DissolveBP != NULL)
	{
		m_pDissolveObject = GetWorld()->SpawnActor(m_DissolveBP.Get(), &GetTransform());
	}
}

// Called when the game starts or when spawned
void ALilypad::BeginPlay()
{
	Super::BeginPlay();
	m_StartPos = GetActorLocation();
}

void ALilypad::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OverlappedComponent == m_pTriggerVolume && OtherComp->IsA<UCapsuleComponent>())
	{
		TArray<UWeightComponent*> weightComps;
		OtherActor->GetComponents(weightComps);
		if (weightComps.Num() > 0)
		{
			for (size_t i = 0; i < weightComps.Num(); i++)
			{
				m_WeightCarried += weightComps[0]->GetWeight();
			}
		}
	}
}

void ALilypad::EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OverlappedComponent == m_pTriggerVolume && OtherComp->IsA<UCapsuleComponent>())
	{
		TArray<UWeightComponent*> weightComps;
		OtherActor->GetComponents(weightComps);
		if (weightComps.Num() > 0)
		{
			for (size_t i = 0; i < weightComps.Num(); i++)
			{
				m_WeightCarried -= weightComps[0]->GetWeight();
			}
		}
	}
}

// Called every frame
void ALilypad::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (m_ToBeDeleted)
	{
		m_DeleteCounter += DeltaTime;
		if (m_DeleteCounter > 1.f)
		{
			Destroy();
		}
		return;
	}

	//change pad location
	auto location = GetActorLocation();
	if (m_WeightCarried > 0)
	{
		m_FloatTimer = 0.f;
		location.Z += m_SinkMultiplier * m_WeightCarried * DeltaTime;
	}
	else
	{
		m_FloatTimer += DeltaTime;
		if (m_FloatTimer > m_FloatDelay)
		{
			location.Z += m_UpwardsForce * DeltaTime;
		}
	}
	SetActorLocation(location);

	//clamp max height
	if (GetActorTransform().GetLocation().Z > m_MaxZHeight)
	{
		location.Z = m_MaxZHeight;
		SetActorLocation(location);
	}

	//check if pad has gone below the near sink height
	if (!m_IsNearSink && GetActorTransform().GetLocation().Z < m_NearSinkZHeight)
	{	
		//loading the particle needs to happen here since the pad's position gets set at some point after BeginPlay is called
		if (m_pNearSinkParticle == nullptr)
		{
			LoadNearSinkParticle();
		}
		m_pNearSinkParticle->ActivateSystem();
		m_IsNearSink = true;
		m_pWarningAudioComponent->Play();	
	}	
	else if (m_IsNearSink)
	{
		//handle shaking
		float randX{ FMath::RandRange(-120.f, 120.f) * DeltaTime };
		float randY{ FMath::RandRange(-120.f, 120.f) * DeltaTime };
		m_pMesh->AddRelativeLocation(FVector(randX, randY, 0.f));

		//check if pad has risen above the near sink height
		if (GetActorLocation().Z > m_NearSinkZHeight)
		{
			m_pNearSinkParticle->Deactivate();
			m_IsNearSink = false;
			//reset x and y location back to start
			FVector newLocation{ m_StartPos.X, m_StartPos.Y, GetActorLocation().Z };
			m_pMesh->SetWorldLocation(newLocation);
		}		
	}

	//destroy pad if it goes below the min height
	if (GetActorTransform().GetLocation().Z < m_MinZHeight)
	{ 
		//play particle
		UNiagaraSystem* pNiagaraSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/RFX/Systems/PS_WaterSplashBig.PS_WaterSplashBig"), nullptr, LOAD_None, nullptr);
		if (pNiagaraSystem->IsValid())
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), pNiagaraSystem, GetActorLocation());
		}

		if (m_pNearSinkParticle != nullptr)
		{
			m_pNearSinkParticle->Deactivate();
		}

		m_pSinkAudioComponent->Play();
		m_ToBeDeleted = true;
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
	}

	//handle dissolve
	if (m_IsDissolving)
	{
		m_DissolveCounter += DeltaTime;		
		if (m_DissolveCounter > m_DissolveDuration)
		{
			m_pDissolveObject->Destroy();
			Destroy();
		}
	}
}

void ALilypad::LoadNearSinkParticle()
{
	UNiagaraSystem* pNiagaraSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/RFX/Systems/PS_WaterSplashSimple.PS_WaterSplashSimple"), nullptr, LOAD_None, nullptr);
	if (pNiagaraSystem->IsValid())
	{
		m_pNearSinkParticle = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), pNiagaraSystem, GetActorLocation());
		m_pNearSinkParticle->Deactivate();
	}
}


