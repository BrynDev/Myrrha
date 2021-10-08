// Fill out your copyright notice in the Description page of Project Settings.


#include "OnDamageCameraShake.h"

UOnDamageCameraShake::UOnDamageCameraShake()
{
    OscillationDuration = 0.08f;
    OscillationBlendInTime = 0.05f;
    OscillationBlendOutTime = 0.05f;

    RotOscillation.Pitch.Amplitude = FMath::RandRange(1.f, 5.f);
    RotOscillation.Pitch.Frequency = FMath::RandRange(5.0f, 10.0f);

    RotOscillation.Yaw.Amplitude = FMath::RandRange(1.f, 5.f);
    RotOscillation.Yaw.Frequency = FMath::RandRange(5.0f, 10.0f);
}