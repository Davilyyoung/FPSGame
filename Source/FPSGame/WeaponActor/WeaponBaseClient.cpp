// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBaseClient.h"

#include "Kismet/GameplayStatics.h"

// Sets default values
AWeaponBaseClient::AWeaponBaseClient()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    //把他作为根组件
	RootComponent = WeaponMesh;
	//客户端上自己能看到　
	WeaponMesh->SetOnlyOwnerSee(true);
	
	
}

void AWeaponBaseClient::DisplayWeaponEffect()
{
	UGameplayStatics::SpawnEmitterAttached(MuzzleFlash,WeaponMesh,TEXT("Fire_FX_Slot"),
		FVector::ZeroVector,FRotator::ZeroRotator,FVector::OneVector,
		EAttachLocation::KeepRelativeOffset,true,EPSCPoolMethod::None,true);
	
	UGameplayStatics::PlaySound2D(GetWorld(),FireSound);
}

