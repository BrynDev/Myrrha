// Fill out your copyright notice in the Description page of Project Settings.


#include "Snake.h"
#include "SnakeSpawner.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "../Lilypad/Lilypad.h"
#include "../Snake/PoisonProjectile.h"
#include "../Fruit/Fruit.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

// Sets default values
ASnake::ASnake()
	: m_Speed{ 1000.f }
	, m_LastDistanceSquared{FLT_MAX}
	, m_ProjectileShot{false}
	, m_IsFleeing{false}
	, m_FleeRadius{2000}
	, m_MinTargettingDistance{2900}
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_pMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("Snake"));
	m_pMesh->SetupAttachment(RootComponent);

	ConstructorHelpers::FObjectFinder<UStaticMesh> snakeAsset(TEXT("StaticMesh'/Game/Characters/Snake/SM_SNake.SM_SNake'"));
	if (snakeAsset.Succeeded())
	{
		m_pMesh->SetStaticMesh(snakeAsset.Object);
	}
	m_pMesh->SetGenerateOverlapEvents(true);
	m_pMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	m_pMesh->OnComponentBeginOverlap.AddDynamic(this, &ASnake::BeginOverlap);

	m_pSpitAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("spitAudioComp"));

	m_pSpitAudioComponent->bAutoActivate = false;

	m_pSpitAudioComponent->SetupAttachment(RootComponent);

	ConstructorHelpers::FObjectFinder<USoundCue> soundAsset(TEXT("'/Game/Audio/SFX/S_Snake_Spit_Cue.S_Snake_Spit_Cue'"));
	if (soundAsset.Succeeded())
	{
		m_pSpitAudioCue = soundAsset.Object;
	}

	if (m_pSpitAudioComponent->IsValidLowLevelFast()) 
	{
		m_pSpitAudioComponent->SetSound(m_pSpitAudioCue);
	}
}

void ASnake::SetSpawner(ASnakeSpawner* spawner)
{
	m_pOwner = spawner;
}

// Called when the game starts or when spawned
void ASnake::BeginPlay()
{
	Super::BeginPlay();

	m_pProjectile = (APoisonProjectile*)GetWorld()->SpawnActor(APoisonProjectile::StaticClass());

	//moving particle
	UNiagaraSystem* pNiagaraSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/RFX/Systems/PS_Snake.PS_Snake"), nullptr, LOAD_None, nullptr);
	if (pNiagaraSystem->IsValid())
	{
		m_pMovingParticleSystem = UNiagaraFunctionLibrary::SpawnSystemAttached(pNiagaraSystem, RootComponent, TEXT("RootComponent"), GetActorLocation(), FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, false);
		m_pMovingParticleSystem->RegisterComponent();
		FAttachmentTransformRules rules{ EAttachmentRule::KeepRelative, false };
		m_pMovingParticleSystem->AttachToComponent(RootComponent, rules);
		//m_pTravelingParticleSystem->Deactivate();
	}
}

void ASnake::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Snake Overlap"));

	if (!m_IsFleeing && OtherActor->IsA<AFruit>())
	{
		UE_LOG(LogTemp, Warning, TEXT("Snake flee"));
		FVector newDestination = GetActorLocation();
		newDestination.Normalize();
		newDestination *= m_FleeRadius;
		m_Destination = newDestination;
		m_IsFleeing = true;
	}
}

// Called every frame
void ASnake::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	auto location = GetActorLocation();
	auto direction = m_Destination - location;
	direction.Normalize();
	SetActorRotation(direction.ToOrientationRotator());
	SetActorLocation(location + direction * m_Speed * DeltaTime);

	if (!m_ProjectileShot)
	{
		if (!m_pTarget->IsValidLowLevel())
		{
			TArray<AActor*> FoundActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALilypad::StaticClass(), FoundActors);
			if (FoundActors.Num() == 0)
			{
				return;
			}

			auto idx = FMath::RandRange(0, FoundActors.Num() - 1); //selecting random lilypad to target

			m_pTarget = (ALilypad*)FoundActors[idx];
		}

		if (m_pProjectile == nullptr || !m_pProjectile->IsValidLowLevel())
		{
			m_pProjectile = (APoisonProjectile*)GetWorld()->SpawnActor(APoisonProjectile::StaticClass());
		}

		float newDistSquared = FVector::DistSquared(GetActorLocation(), m_pTarget->GetActorLocation());
		if (!m_IsFleeing && !m_pProjectile->IsActive() && newDistSquared > m_LastDistanceSquared)
		{
			m_pProjectile->ShootAtTarget(m_pTarget, GetActorLocation());
			m_ProjectileShot = true;
		}
		m_LastDistanceSquared = newDistSquared;
	}
	
	if (FVector::DistSquared(location, m_Destination) < 100.f)
	{
		m_pOwner->DeactivateSnake(this);
	}
}

void ASnake::SetDestination(const FVector& destination)
{
	m_Destination = destination;

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALilypad::StaticClass(), FoundActors);
	

	for (int i = 0; i < FoundActors.Num(); i++)
	{
		auto distSquared = FVector::DistSquared(FoundActors[i]->GetActorLocation(), GetActorLocation());
		UE_LOG(LogTemp, Warning, TEXT("%f"), distSquared);
		if (distSquared < m_MinTargettingDistance*m_MinTargettingDistance)
		{
			UE_LOG(LogTemp, Warning, TEXT("Lilypad to close to snake, not targetable"));
			FoundActors[i] = FoundActors[FoundActors.Num()-1];
			FoundActors.Pop();
			UE_LOG(LogTemp, Warning, TEXT("%d"), int(FoundActors.Num()));

		}
	}

	if (FoundActors.Num() == 0)
	{
		return;
	}

	auto idx = FMath::RandRange(0, FoundActors.Num() - 1); //selecting random lilypad to target


	m_pTarget = (ALilypad*)FoundActors[idx];

	UNiagaraSystem* pNiagaraSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/RFX/Systems/PS_LandIndication.PS_LandIndication"), nullptr, LOAD_None, nullptr);
	if (pNiagaraSystem->IsValid())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), pNiagaraSystem, m_pTarget->GetActorLocation());
	}
	m_ProjectileShot = false;
	m_IsFleeing = false;
	m_LastDistanceSquared = FLT_MAX;
}

