// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuCharacter.h"

// Sets default values
AMenuCharacter::AMenuCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMenuCharacter::BeginPlay()
{
	Super::BeginPlay();

	//camera fade in, needs to be done from the player to access the controller
	APlayerController* pPlayerController = Cast<APlayerController>(Controller);
	pPlayerController->ClientSetCameraFade(true, FColor::Black, FVector2D(1.0, 0.0), 1.5f);
}

// Called every frame
void AMenuCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMenuCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

