// Fill out your copyright notice in the Description page of Project Settings.


#include "PoisonProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "../Lilypad/Lilypad.h"
#include "Components/CapsuleComponent.h"
#include "../Fruit/Fruit.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

// Sets default values
APoisonProjectile::APoisonProjectile()
	: m_Speed{300.f}
	, m_Active{false}
	, m_pTarget{nullptr}
	, m_pCollision{nullptr}
	, m_DissolveBP{NULL}
	, m_pDissolveObject{nullptr}
	, m_Velocity{}
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//collision component
	m_pCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Collision"));
	m_pCollision->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	m_pCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	m_pCollision->SetGenerateOverlapEvents(true);
	m_pCollision->SetSimulatePhysics(false);
	m_pCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	m_pCollision->SetCapsuleSize(100.f, 130.f);

	RootComponent = m_pCollision;

	//dissolve blueprint
	/*ConstructorHelpers::FObjectFinder<UClass> dissolveBP(TEXT("Class'/Game/BP_Dissolve.BP_Dissolve_C'"));
	if (dissolveBP.Object)
	{
		m_DissolveBP = dissolveBP.Object;
	}	*/
}

void APoisonProjectile::ShootAtTarget(AActor* Target, const FVector& startPos)
{
	if (!Target->IsValidLowLevel())
	{
		return;
	}

	SetActorLocation(startPos);
	m_pTarget = Target;
	SetActorHiddenInGame(false);
	SetActorTickEnabled(true);
	SetActorEnableCollision(true);

	m_Active = true;	
	m_pTravelingParticleSystem->ActivateSystem();

	//float secsToHitLilypad = sqrtf((2*(startPos.Z- Target->GetActorLocation().Z))/(m_Gravity/100.f)); //free fall calculation for movement in an arc, doesnt work for some reason

	//float distXY = FVector::Dist2D(startPos, Target->GetActorLocation()); 

	//float speedXY = distXY / secsToHitLilypad;

	FVector direction{ Target->GetActorLocation() - startPos };
	direction.Normalize();
	direction *= m_Speed;

	m_Velocity = direction;
	
}

bool APoisonProjectile::IsActive()
{
	return m_Active;
}

void APoisonProjectile::Neutralized()
{
	UNiagaraSystem* pNiagaraSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/RFX/Systems/PS_PoisonLanded.PS_PoisonLanded"), nullptr, LOAD_None, nullptr);
	if (pNiagaraSystem->IsValid())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), pNiagaraSystem, GetActorLocation());
	}

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);
	m_Active = false;
	m_pTravelingParticleSystem->Deactivate();	
}

// Called when the game starts or when spawned
void APoisonProjectile::BeginPlay()
{
	Super::BeginPlay();
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);

	//traveling particle
	UNiagaraSystem* pNiagaraSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/RFX/Systems/PS_Poison.PS_Poison"), nullptr, LOAD_None, nullptr);
	if (pNiagaraSystem->IsValid())
	{
		m_pTravelingParticleSystem = UNiagaraFunctionLibrary::SpawnSystemAttached(pNiagaraSystem, RootComponent, TEXT("RootComponent"), GetActorLocation(), FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, false);
		m_pTravelingParticleSystem->RegisterComponent();
		FAttachmentTransformRules rules{ EAttachmentRule::KeepRelative, false };
		m_pTravelingParticleSystem->AttachToComponent(RootComponent, rules);
		m_pTravelingParticleSystem->Deactivate();
	}

	/*if (m_DissolveBP != NULL)
	{
		m_pDissolveObject = GetWorld()->SpawnActor(m_DissolveBP.Get());		
		m_pDissolveObject->RegisterAllComponents();
		FAttachmentTransformRules rules{ EAttachmentRule::SnapToTarget, false };
		//m_pDissolveObject->AttachToComponent(RootComponent, rules);
		m_pDissolveObject->AttachToActor(this, rules);
		m_pDissolveObject->SetActorHiddenInGame(true);
		
	}*/

}

// Called every frame
void APoisonProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	auto location = GetActorLocation();
	location += m_Velocity * DeltaTime;

	SetActorLocation(location);
	//m_pDissolveObject->SetActorLocation(location);

	if (m_pTarget != nullptr && GetActorLocation().Z < 40)
	{
		((ALilypad*)m_pTarget)->Dissolve();
		/*SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
		SetActorTickEnabled(false);
		m_Active = false;*/
		Neutralized();
	}
}

