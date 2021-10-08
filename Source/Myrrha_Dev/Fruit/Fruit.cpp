// Fill out your copyright notice in the Description page of Project Settings.


#include "Fruit.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "FruitSpawner.h"
#include "../WeightComponent/WeightComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "../MainGameMode.h"
#include "../Snake/PoisonProjectile.h"



// Sets default values
AFruit::AFruit()
	: m_Weight{5}
	, m_WaterLevel{3.f}
	, m_DespawnDistance{3000.f}
	, m_DefaultUpAccel{ 750000000.f }
	, m_ThrownUpAccel{ 650000000.f }
	, m_pSpawner{nullptr}
	, m_CurrentUpAccel{0}
	, m_WasThrown{false}
	, m_WasHit{false}
	, m_PointsWater{15.f}
	, m_PointsPoison{50.f}
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//component setup
	//collision component
	m_pCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Collision"));
	m_pCollision->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	m_pCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	m_pCollision->SetNotifyRigidBodyCollision(true); //generate hit events
	m_pCollision->SetSimulatePhysics(true);
	m_pCollision->SetEnableGravity(true);
	m_pCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	
	RootComponent = m_pCollision;

	//static mesh component
	m_pStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	m_pStaticMesh->SetupAttachment(RootComponent);
	m_pStaticMesh->SetEnableGravity(false);
	m_pStaticMesh->SetCastShadow(false);
	m_pStaticMesh->SetSimulatePhysics(false);
	m_pStaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//weight component
	m_pWeightComp = CreateDefaultSubobject<UWeightComponent>(TEXT("Weight"));
	
	
	//audio
	m_pWaterAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("waterAudioComp"));

	m_pWaterAudioComponent->bAutoActivate = false;

	m_pWaterAudioComponent->SetupAttachment(RootComponent);

	ConstructorHelpers::FObjectFinder<USoundCue> soundAsset(TEXT("SoundCue'/Game/Audio/SFX/S_Water_Cue.S_Water_Cue'"));
	if (soundAsset.Succeeded())
	{
		m_pWaterAudioCue = soundAsset.Object;
	}

	if (m_pWaterAudioComponent->IsValidLowLevelFast()) {
		m_pWaterAudioComponent->SetSound(m_pWaterAudioCue);
	}

	m_pCollisionAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("collisionAudioComp"));

	m_pCollisionAudioComponent->bAutoActivate = false;

	m_pCollisionAudioComponent->SetupAttachment(RootComponent);

	ConstructorHelpers::FObjectFinder<USoundCue> soundAsset2(TEXT("SoundCue'/Game/Audio/SFX/S_Fruit_Hit_Cue.S_Fruit_Hit_Cue'"));
	if (soundAsset.Succeeded())
	{
		m_pCollisionAudioCue = soundAsset2.Object;
	}

	if (m_pCollisionAudioComponent->IsValidLowLevelFast()) 
	{
		m_pCollisionAudioComponent->SetSound(m_pCollisionAudioCue);
	}

	//set upwards acceleration
	m_CurrentUpAccel = m_DefaultUpAccel;
}

// Called when the game starts or when spawned
void AFruit::BeginPlay()
{
	Super::BeginPlay();

	m_pCollision->SetMassScale(NAME_None, 2000.f);
	m_pWeightComp->SetWeight(m_Weight);

	m_pCollision->OnComponentHit.AddDynamic(this, &AFruit::OnHit);
	m_pCollision->OnComponentBeginOverlap.AddDynamic(this, &AFruit::BeginOverlap);


	//niagara component, needs to be done here for GetActorLocation to work
	UNiagaraSystem* pNiagaraSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/RFX/Systems/PS_Butterfly.PS_Butterfly"), nullptr, LOAD_None, nullptr);
	if (pNiagaraSystem->IsValid())
	{
		m_pCarriedParticleSystem = UNiagaraFunctionLibrary::SpawnSystemAttached(pNiagaraSystem, RootComponent, TEXT("RootComponent"), GetActorLocation(), FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, false);
		m_pCarriedParticleSystem->RegisterComponent();
		FAttachmentTransformRules rules{ EAttachmentRule::KeepRelative, false };
		m_pCarriedParticleSystem->AttachToComponent(RootComponent, rules);
		m_pCarriedParticleSystem->Deactivate();
	}
}

void AFruit::Deactivate()
{
	m_WasThrown = false;
	m_CurrentUpAccel = m_DefaultUpAccel; //in case the fruit got thrown before deactivating, reset upwards acceleration back to normal

	if (m_pSpawner != nullptr)
	{			
		m_pSpawner->DeactivateFruit(this);
	}
	else
	{
		Destroy();
	}

	//shouldn't be active by this point anyways, but just in case
	if (m_pCarriedParticleSystem->IsActive())
	{
		m_pCarriedParticleSystem->Deactivate();
	}
	
}

// Called every frame
void AFruit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	m_pCollision->AddForce(FVector(0, 0, m_CurrentUpAccel));

	/*FVector velocity = m_pCollision->GetPhysicsLinearVelocity();
	velocity.Z += m_CurrentUpAccel * DeltaTime;
	m_pCollision->SetPhysicsLinearVelocity(velocity);*/
	
	if (FVector::DistSquared(GetActorLocation(), FVector{ 0.f,0.f,0.f }) > m_DespawnDistance*m_DespawnDistance)
	{
		if (m_WasThrown || m_WasHit)
		{
			auto gameMode = Cast<AMainGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
			if (gameMode != nullptr)
			{
				gameMode->AddScore(m_PointsWater);
				OnGiveScoreWater();
			}
		}
		Deactivate();
	}
	else if (GetActorLocation().Z < m_WaterLevel)
	{
		//fell in water, play splash particle
		UNiagaraSystem* pNiagaraSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/RFX/Systems/PS_WaterSplashBig.PS_WaterSplashBig"), nullptr, LOAD_None, nullptr);
		m_pWaterAudioComponent->Play();
		if (pNiagaraSystem->IsValid())
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), pNiagaraSystem, GetActorLocation());
		}
		if (m_WasThrown || m_WasHit)
		{
			auto gameMode = Cast<AMainGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
			if (gameMode != nullptr)
			{
				gameMode->AddScore(m_PointsWater);
				OnGiveScoreWater();
			}
		}
		Deactivate();
	}	
}

void AFruit::HandleGrab()
{
	m_pCollision->SetPhysicsLinearVelocity(FVector(0, 0, 0));
	m_CurrentUpAccel = 0.f; //stop fruit from drifting upwards
	m_pCollision->SetEnableGravity(false); //set gravity to off so the fruit doesn't land on the character's head	

	m_pCarriedParticleSystem->ActivateSystem();
}

void AFruit::HandleThrow(const FVector& throwVector)
{
	m_pCollision->SetEnableGravity(true);
	m_CurrentUpAccel = m_ThrownUpAccel; //restore (some) gravity	
	m_pCollision->SetPhysicsLinearVelocity(throwVector);
	m_WasThrown = true;

	m_pCarriedParticleSystem->Deactivate();
}

void AFruit::SetStaticMesh(UStaticMesh* pStaticMesh)
{
	m_pStaticMesh->SetStaticMesh(pStaticMesh);
}

void AFruit::SetRandomAngularVelocity()
{
	m_pCollision->SetPhysicsAngularVelocityInDegrees(FVector{ FMath::FRandRange(-60, 60), FMath::FRandRange(-60, 60), FMath::FRandRange(-60, 60)});
}

void AFruit::SetSpawner(AFruitSpawner* spawner)
{
	m_pSpawner = spawner;
}

void AFruit::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	//only execute on the fruit thrown by the player
	if (OtherActor->IsA<AFruit>() && m_WasThrown)
	{
		AFruit* hitFruit{ Cast<AFruit>(OtherActor) };
		//set velocity of the hit fruit
		FVector newVel{ GetVelocity() };
		newVel.Z += 1300.f;
		hitFruit->m_pCollision->SetPhysicsLinearVelocity(newVel);
		m_WasThrown = false; //immediately set to false, to make sure this OnHit only gets called once per collision
		m_WasHit = true;
		hitFruit->m_WasHit = true;
		UNiagaraSystem* pNiagaraSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/RFX/Systems/PS_ThrowImpact.PS_ThrowImpact"), nullptr, LOAD_None, nullptr);
		if (pNiagaraSystem->IsValid())
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), pNiagaraSystem, Hit.ImpactPoint);
		}
		m_pCollisionAudioComponent->Play();
	}
	
}

void AFruit::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->IsA<APoisonProjectile>())
	{
		auto gameMode = Cast<AMainGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
		gameMode->AddScore(m_PointsPoison);
		UE_LOG(LogTemp, Warning, TEXT("fruit poison overlap"));
		Cast<APoisonProjectile>(OtherActor)->Neutralized();
		OnGiveScorePoison();
	}
}

