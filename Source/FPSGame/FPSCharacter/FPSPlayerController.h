// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FPSGame/WeaponActor/WeaponBaseClient.h"
#include "GameFramework/PlayerController.h"
#include "FPSPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class FPSGAME_API AFPSPlayerController : public APlayerController
{
	GENERATED_BODY()
public:

	void PlayerCameraShake(TSubclassOf<UCameraShakeBase> CameraShake);

	//蓝图里实现比较方便 显示UI
	UFUNCTION(Category="PlayerUI",BlueprintImplementableEvent)
	void CreatePlayerUI();

	//蓝图里实现比较方便 十字扩散
	UFUNCTION(Category="PlayerUI",BlueprintImplementableEvent)
	void DoCrosshairRecoil();

	//蓝图里实现比较方便 更改AmmoUI
	UFUNCTION(Category="PlayerUI",BlueprintImplementableEvent)
	void UpdateAmmoUI(int32 ClipCurrentAmmo, int32 GunCurrentAmmo);

	//蓝图里实现比较方便 更改HeathUI
	UFUNCTION(Category="PlayerUI",BlueprintImplementableEvent)
	void UpdateHeathUI(float NewHealth);

	
};

