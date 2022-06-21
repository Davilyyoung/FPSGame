// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSPlayerController.h"


void AFPSPlayerController::PlayerCameraShake(TSubclassOf<UCameraShakeBase> CameraShake)
{
	
	ClientPlayCameraShake(CameraShake,1,ECameraShakePlaySpace::CameraLocal,FRotator::ZeroRotator);
}
