// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponBaseClient.generated.h"

//在客户端
UCLASS()
class FPSGAME_API AWeaponBaseClient : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponBaseClient();

	//有可能要把自己的模型放上去 所以Eidt要能够编辑
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	USkeletalMeshComponent* WeaponMesh;
	
	//这里只给一个方法 留给蓝图来实现 要去播放枪体动画 在蓝图中可以直接拖 C++中需要加载 很麻烦
	UFUNCTION(BlueprintImplementableEvent,Category="FPGunAnimation")
	void PlayShootAnimation();
	//这里只给一个方法 留给蓝图来实现 要去播放枪体动画 在蓝图中可以直接拖 C++中需要加载 很麻烦
	UFUNCTION(BlueprintImplementableEvent,Category="FPGunAnimation")
	void PlayReloadAnimation();

	
	UPROPERTY(EditAnywhere)
	UAnimMontage* ClientArmsFireAnimMontage;

	UPROPERTY(EditAnywhere)
	UAnimMontage* ClientArmsReloadAnimMontage;
	
	UPROPERTY(EditAnywhere)
	USoundBase* FireSound;

	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UCameraShakeBase> CameraShakeClass;

	//混合的动画是那个数字
	UPROPERTY(EditAnywhere)
	int32 FPArmsBlendPose;

	
	
	void DisplayWeaponEffect();
};
