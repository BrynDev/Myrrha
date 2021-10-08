// Fill out your copyright notice in the Description page of Project Settings.


#include "WeightComponent.h"

// Sets default values for this component's properties
UWeightComponent::UWeightComponent()
	: m_Weight{5}
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UWeightComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UWeightComponent::SetWeight(int weight)
{
	m_Weight = weight;
}

int UWeightComponent::GetWeight()
{
	return m_Weight;
}

// Called every frame
void UWeightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

