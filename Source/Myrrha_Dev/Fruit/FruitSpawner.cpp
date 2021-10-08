// Fill out your copyright notice in the Description page of Project Settings.


#include "FruitSpawner.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "../Lilypad/Lilypad.h"
#include "Components/CapsuleComponent.h"
#include "Fruit.h"
#include "Math/UnrealMathUtility.h"
#include "../MainGameMode.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"

// Sets default values
AFruitSpawner::AFruitSpawner()
	: m_FruitSpawnCounter{0}
	, m_FruitSpawnDelay{10.f}
	, m_MinFruitSpawnDelay{4.5f}
	, m_SecondsBetweenDelayDecrease{90}
	, m_StartBufferSeconds{30}
	, m_DelayDecreaseAmount{1.8f}
	, m_FruitSpawnHeight{800.f}
	, m_FruitPoolSize{20}
	, m_ActiveFruits{0}
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	ConstructorHelpers::FObjectFinder<UClass> fruitBlueprint(TEXT("Class'/Game/BP_Fruit.BP_Fruit_C'"));
	if (fruitBlueprint.Object)
	{
		m_FruitBlueprint = fruitBlueprint.Object;
	}

	ConstructorHelpers::FObjectFinder<UStaticMesh> fruitAsset1(TEXT("StaticMesh'/Game/Environment/Meshes/SM_Fruit_01.SM_Fruit_01'"));
	if (fruitAsset1.Succeeded())
	{
		m_SMFruit_01 = fruitAsset1.Object;
	}

	ConstructorHelpers::FObjectFinder<UStaticMesh> fruitAsset2(TEXT("StaticMesh'/Game/Environment/Meshes/SM_Fruit_02.SM_Fruit_02'"));
	if (fruitAsset2.Succeeded())
	{
		m_SMFruit_02 = fruitAsset2.Object;
	}

	ConstructorHelpers::FObjectFinder<UStaticMesh> fruitAsset3(TEXT("StaticMesh'/Game/Environment/Meshes/SM_Fruit_03.SM_Fruit_03'"));
	if (fruitAsset3.Succeeded())
	{
		m_SMFruit_03 = fruitAsset3.Object;
	}

	m_pSpawnAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("spawnAudioComp"));

	m_pSpawnAudioComp->bAutoActivate = false;

	m_pSpawnAudioComp->SetupAttachment(RootComponent);

	ConstructorHelpers::FObjectFinder<USoundCue> soundAsset(TEXT("'/Game/Audio/SFX/S_Fruit_Spawn_Cue.S_Fruit_Spawn_Cue'"));
	if (soundAsset.Succeeded())
	{
		m_pSpawnAudioCue = soundAsset.Object;
	}

	if (m_pSpawnAudioComp->IsValidLowLevelFast()) {
		m_pSpawnAudioComp->SetSound(m_pSpawnAudioCue);
	}
}
// Called when the game starts or when spawned
void AFruitSpawner::BeginPlay()
{
	Super::BeginPlay();


	
	auto world = GetWorld();
	m_FruitArray.Reserve(m_FruitPoolSize);
	for (int i = 0; i < m_FruitPoolSize; i++)
	{
		auto fruit = world->SpawnActor(m_FruitBlueprint.Get());
		fruit->SetActorHiddenInGame(true);
		fruit->SetActorEnableCollision(false);
		fruit->SetActorTickEnabled(false);
		((AFruit*)fruit)->SetSpawner(this);
		switch (i%3)
		{
		case 0:
			((AFruit*)fruit)->SetStaticMesh(m_SMFruit_01);
			break;
		case 1:
			((AFruit*)fruit)->SetStaticMesh(m_SMFruit_02);
			break;
		case 2:
			((AFruit*)fruit)->SetStaticMesh(m_SMFruit_03);
			break;
		default:
			break;
		}
		m_FruitArray.Push(fruit);
	}
}

// Called every frame
void AFruitSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	m_FruitSpawnCounter += DeltaTime;
	while (m_FruitSpawnCounter > m_FruitSpawnDelay)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnFruit"));

		m_FruitSpawnCounter -= m_FruitSpawnDelay;

		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALilypad::StaticClass(), FoundActors);
		if (FoundActors.Num() == 0)
		{
			break;
		}

		auto idx = FMath::RandRange(0, FoundActors.Num()-1); //selecting random lilypad to spawn above

		TArray<UStaticMeshComponent*> Components;
		FoundActors[idx]->GetComponents<UStaticMeshComponent>(Components);
		FBox boundingBox = Components[0]->GetStaticMesh()->GetBoundingBox(); //getting the mesh to get the size for offset from center
		FVector center;
		FVector extents;
		boundingBox.GetCenterAndExtents(center, extents); //center is in local space

		center.X += FoundActors[idx]->GetActorLocation().X;
		center.Y += FoundActors[idx]->GetActorLocation().Y;

		center.X += FMath::RandRange(0.f, extents.X) - extents.X/2.f; //random offset inside the bounds
		center.Y += FMath::RandRange(0.f, extents.Y) - extents.Y/2.f;
		
		center.Z = m_FruitSpawnHeight;

		SpawnFruit(center);
		UpdateSpawnDelay();
		UNiagaraSystem* pNiagaraSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/RFX/Systems/PS_LandIndication.PS_LandIndication"), nullptr, LOAD_None, nullptr);
		if (pNiagaraSystem->IsValid())
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), pNiagaraSystem, FoundActors[idx]->GetActorLocation());
		}
	}
	
}

void AFruitSpawner::SpawnFruit(const FVector& location)
{
	m_pSpawnAudioComp->Play();
	if (m_ActiveFruits == m_FruitPoolSize)
	{
		UE_LOG(LogTemp, Warning, TEXT("Increase ObjectPool size"))
		auto world = GetWorld();
		m_FruitPoolSize *= 2;
		m_FruitArray.Reserve(m_FruitPoolSize); //when array runs out of space double the capacity, shouldnt happen but this is a safety measure just in case
		for (int i = 0; i < m_FruitPoolSize/2; i++)
		{
			auto fruit = world->SpawnActor(m_FruitBlueprint.Get());
			fruit->SetActorHiddenInGame(true);
			fruit->SetActorEnableCollision(false);
			fruit->SetActorTickEnabled(false);
			((AFruit*)fruit)->SetSpawner(this);
			m_FruitArray.Push(fruit);
		}
	}
	if (m_FruitArray[m_ActiveFruits] == nullptr || !m_FruitArray[m_ActiveFruits]->IsValidLowLevel())
	{
		m_FruitArray[m_ActiveFruits] = GetWorld()->SpawnActor(m_FruitBlueprint.Get());
		((AFruit*)m_FruitArray[m_ActiveFruits])->SetSpawner(this);
		switch (m_ActiveFruits%3)
		{
		case 0:
			((AFruit*)m_FruitArray[m_ActiveFruits])->SetStaticMesh(m_SMFruit_01);
			break;
		case 1:
			((AFruit*)m_FruitArray[m_ActiveFruits])->SetStaticMesh(m_SMFruit_02);
			break;
		case 2:
			((AFruit*)m_FruitArray[m_ActiveFruits])->SetStaticMesh(m_SMFruit_03);
			break;
		}
	}
	m_FruitArray[m_ActiveFruits]->SetActorLocation(location);
	m_FruitArray[m_ActiveFruits]->SetActorHiddenInGame(false);
	m_FruitArray[m_ActiveFruits]->SetActorEnableCollision(true);
	m_FruitArray[m_ActiveFruits]->SetActorTickEnabled(true); 
	((AFruit*)m_FruitArray[m_ActiveFruits])->SetRandomAngularVelocity();
	TArray<UCapsuleComponent*> capsules;
	((AFruit*)m_FruitArray[m_ActiveFruits])->GetComponents(capsules);
	for (size_t i = 0; i < capsules.Num(); i++)
	{
		capsules[i]->SetPhysicsLinearVelocity(FVector{ 0.f, 0.f, 0.f });
	}
	++m_ActiveFruits;


	UNiagaraSystem* pNiagaraSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/RFX/Systems/PS_Leaves.PS_Leaves"), nullptr, LOAD_None, nullptr);
	if (pNiagaraSystem->IsValid())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), pNiagaraSystem, location);
	}
}

void AFruitSpawner::DeactivateFruit( AFruit* actor)
{
	for (int i = 0; i < m_ActiveFruits; i++)
	{
		if (m_FruitArray[i] == actor)
		{
			actor->SetActorHiddenInGame(true);
			actor->SetActorEnableCollision(false);
			actor->SetActorTickEnabled(false);
			AActor* temp = m_FruitArray[m_ActiveFruits - 1];
			m_FruitArray[m_ActiveFruits - 1] = actor;
			m_FruitArray[i] = temp;
			--m_ActiveFruits;
		}
	}
}

void AFruitSpawner::UpdateSpawnDelay()
{
	AMainGameMode* pGameMode{ GetWorld()->GetAuthGameMode<AMainGameMode>() };
	int survivedMinutes{ pGameMode->GetSurvivedMinutes() };
	int survivedSeconds{ pGameMode->GetSurvivedSeconds() };
	int totalSeconds{ survivedSeconds + 60 * survivedMinutes };

	//if delay has reached minimum value or the delay cannot start decreasing yet, early exit
	if (m_FruitSpawnDelay <= m_MinFruitSpawnDelay || totalSeconds < m_StartBufferSeconds)
	{
		return;
	}
	
	if (totalSeconds % m_SecondsBetweenDelayDecrease == 0)
	{
		m_FruitSpawnDelay -= m_DelayDecreaseAmount;

		//clamp the value if necessary
		if (m_FruitSpawnDelay < m_MinFruitSpawnDelay)
		{
			m_FruitSpawnDelay = m_MinFruitSpawnDelay;
		}
	}
}