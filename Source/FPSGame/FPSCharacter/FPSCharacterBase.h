// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "FPSPlayerController.h"
#include "Camera/CameraComponent.h"
#include "FPSGame/WeaponActor/WeaponBaseServer.h"
#include "GameFramework/Character.h"
#include "FPSCharacterBase.generated.h"

UCLASS()
class FPSGAME_API AFPSCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:

	AFPSCharacterBase();

private:

#pragma region Component // 折叠宏 组件代码

	//meta =(AllowPrivateAccess = "true") privateのアクセスできます。使蓝图可以访问私有变量
	UPROPERTY(Category = Character,VisibleAnywhere,BlueprintReadOnly, meta =(AllowPrivateAccess = "true"))
	UCameraComponent* PlayerCamera;

	//腕を置く
	UPROPERTY(Category = Character,VisibleAnywhere,BlueprintReadOnly, meta =(AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FPArmsMesh;

	UPROPERTY(Category = Character,BlueprintReadOnly, meta =(AllowPrivateAccess = "true"))
	UAnimInstance* ClientArmsAnimBP;

	UPROPERTY(Category = Character,BlueprintReadOnly, meta =(AllowPrivateAccess = "true"))
	UAnimInstance* ServerBodysAnimBP;

	//要标记UPROPERTY 或者智能指针 不然会被垃圾回收
	UPROPERTY(BlueprintReadOnly, meta =(AllowPrivateAccess = "true"))
	AFPSPlayerController* FPSPlayerController;

	UPROPERTY(EditAnywhere)
	EWeaponType TestStartWeapon;
public:
	UFUNCTION()
	void DelayBeginPlayCallBack();

#pragma endregion 

protected:
	
	virtual void BeginPlay() override;

#pragma region InputEvent // 折叠宏 输入相关绑定
	//子类可能还要去方法 或者我们要他成虚方法 其他类不能知道我们的方法所以protected里
	void MoveForWorld(float AxisValue);
	void MoveRight(float AxisValue);
	void JumpAction();
	void StopJumpAction();
	void LowSpeedWalkAction();
	void NormalSpeedWalkAction();
	void InputFirePressed();
	void InputFireReleased();
	void InputReload();
	void InputAimingPressed();
	void InputAimingReleased();
#pragma endregion

#pragma region Fire // 折叠宏 射击相关 换弹和设计都写在这里
public:

	//计时器
	FTimerHandle AutomaticFireTimerHandle;
	void AutomaticFire();

	// 后坐力
	float NewVerticalRecoilAmount;//新的点
	float OldVerticalRecoilAmount;//上一个点
	float VerticalRecoilAmount;//减的值
	float RecoilXCoordPerShoot;//每次射击时候的X坐标
	void ResetRecoil();
	float NewHorizontalRecoilAmount;//新的点
	float OldHorizontalRecoilAmount;//上一个点
	float HorizontalRecoilAmount;//减的值

	float PistolSpreadMin = 0.0f; //手枪的后坐力算法 最小偏移
	float PistolSpreadMax = 0.0f; //手枪的后坐力算法 最大偏移
	
	
	//步枪相关
	void FireWeaponPrimary();
	void StopFirePrimary();
	void RilfeLineTrace(FVector CameraLocktion,FRotator CameraRotaion,bool IsMoving);

	//手枪相关
	void FireWeaponSecondary();
	void StopFireSecondary();
	void PistolLineTrace(FVector CameraLocktion,FRotator CameraRotaion,bool IsMoving);
	//手枪散射武器延迟CallBack
	UFUNCTION()
	void DelaySpreadWeaponShootCallBack();

	//狙击枪相关
	void FireSniperPrimary();
	void StopFireSniperPrimary();
	void SniperLineTrace(FVector CameraLocation,FRotator CameraRotaion,bool IsMoving);
	UPROPERTY(Replicated)
	bool IsAiming;
	UFUNCTION()
	void DelaySniperShootCallBack();
	UPROPERTY(VisibleAnywhere,Category="SniperUI")
	UUserWidget* WidgetScope;//镜
	UPROPERTY(EditAnywhere,Category="SniperUI")
	TSubclassOf<UUserWidget> SniperScopeBPClass; //镜UI蓝图类 需要build UMG


	

	//换弹夹
	UPROPERTY(Replicated)//Replicated 服务器上变了 客户端要变
	bool IsFire;
	UPROPERTY(Replicated)//Replicated 服务器上变了 客户端要变
	bool IsReloading;
	//延时
	UFUNCTION()
	void DelayPlayArmReloadCallBack();
	
	
	//子弹射击相关
	void DemagePlayer(EPhysicalSurface Surface, AActor* DamagedActor,FVector& HitFromDirection,FHitResult& HitInfo);//伤害玩家

	UFUNCTION()
	void OnHit(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser);

	//角色生命值
	float Health;

	//死亡方法            //打我的人是谁
	void DeathShootDeath(AActor* DamageActor);


#pragma endregion

#pragma region Weapon // 折叠宏 玩家和武器相关

public:
	//主武器的指针需要存下来 不然会被垃圾回收
	
	//主武器   被碰到的时候把自己传进来   把武器装备起来
	void EquipPrimary(AWeaponBaseServer* WeaponBaseServer);

	//副武器   被碰到的时候把自己传进来   把武器装备起来
	void EquipSecondary(AWeaponBaseServer* WeaponBaseServer);

	//蓝图实现持枪动作更新
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateFPArmsBlendPose(int32 NewIndex);

private:
	//现在使用的枪是那把枪
	UPROPERTY(meta = (AllowPrivateAccess = "true"),Replicated)//Test用
	EWeaponType ActiveWeapon;
	
	//存服务器上的武器  主武器
	UPROPERTY(meta =(AllowPrivateAccess = "true"))
	AWeaponBaseServer* ServerPrimryWeapon;
	
	//存服务器上的武器  副武器
	UPROPERTY(meta =(AllowPrivateAccess = "true"))
	AWeaponBaseServer* ServerSecondaryWeapon;

	//存客户端上的武器  主武器
	UPROPERTY(meta =(AllowPrivateAccess = "true"))
	AWeaponBaseClient* ClientPrimaryWeapon;

	//存客户端上的武器 副武器
	UPROPERTY(meta =(AllowPrivateAccess = "true"))
	AWeaponBaseClient* ClientSecondaryWeapon;

	//玩家开始就持有枪械
	void StartWithKindOfWeapon();
	//购买枪械
	void PurChaseWeapon(EWeaponType WeaponType);

	AWeaponBaseClient* GetCurrentClintFPArmsWeaponActor();
	AWeaponBaseServer* GetCurrentServerTPBodysWeaponActor();

#pragma endregion

public:	
	
	virtual void Tick(float DeltaTime) override;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	
//网络关于服务器的东西卸载public里
public:
#pragma region NetWorking // 折叠宏 服务器相关
	//Server服务器上上运行，
	//Reliable这个函数是绝对可行，从客户端到服务器的这条路线绝对可信，一定能在服务器上执行结束，不会丢包。一些重要的东西加Reliable，不重要的可以不加
	//WithValidation 会给这个函数加一个条件 返回为false的时网络候链接会断开，防止作弊
	UFUNCTION(Server,Reliable,WithValidation)
    void ServerLowSpeedWalkAction();//调用用这个
	void ServerLowSpeedWalkAction_Implementation();//实现在这里
	bool ServerLowSpeedWalkAction_Valiedate();//返回false会断开链接

	UFUNCTION(Server,Reliable,WithValidation)
	void ServerNormalSpeedWalkAction();//调用用这个
	void ServerNormalSpeedWalkAction_Implementation();//实现在这里	
	bool ServerNormalSpeedWalkAction_Valiedate();//返回false会断开链接

	UFUNCTION(Server,Reliable,WithValidation)
	void ServerFireRifleWeapon(FVector CameraLocktion,FRotator CameraRotation,bool IsMoving);//调用用这个
	void ServerFireRifleWeapon_Implementation(FVector CameraLocation,FRotator CameraRotation,bool IsMoving);//实现在这里
	bool ServerFireRifleWeapon_Valiedate(FVector CameraLocktion,FRotator CameraRotion,bool IsMoving);//返回false会断开链接

	UFUNCTION(Server,Reliable,WithValidation)
	void ServerFirePistolWeapon(FVector CameraLocktion,FRotator CameraRotion,bool IsMoving);//调用用这个
	void ServerFirePistolWeapon_Implementation(FVector CameraLocation,FRotator CameraRotation,bool IsMoving);//实现在这里
	bool ServerFirePistolWeapon_Valiedate(FVector CameraLocktion,FRotator CameraRotion,bool IsMoving);//返回false会断开链接

	UFUNCTION(Server,Reliable,WithValidation)
	void ServerFireSniperWeapon(FVector CameraLocktion,FRotator CameraRotion,bool IsMoving);//调用用这个
	void ServerFireSniperWeapon_Implementation(FVector CameraLocation,FRotator CameraRotation,bool IsMoving);//实现在这里
	bool ServerFireSniperWeapon_Valiedate(FVector CameraLocktion,FRotator CameraRotion,bool IsMoving);//返回false会断开链接

	
	UFUNCTION(Server,Reliable,WithValidation)
	void ServerReloadPrimary();//调用用这个
	void ServerReloadPrimary_Implementation();//实现在这里
	bool ServerReloadPrimary_Valiedate();//返回false会断开链接

	UFUNCTION(Server,Reliable,WithValidation)
	void ServerReloadSecondary();//调用用这个
	void ServerReloadSecondary_Implementation();//实现在这里
	bool ServerReloadSecondary_Valiedate();//返回false会断开链接

	UFUNCTION(Server,Reliable,WithValidation)
    void ServerStopFire();//调用用这个
	void ServerStopFire_Implementation();//实现在这里
	bool ServerStopFire_Valiedate();//返回false会断开链接
	
	UFUNCTION(Server,Reliable,WithValidation)
	void ServerSetAiming(bool AimingState);//调用用这个
	void ServerSetAiming_Implementation(bool AimingState);//实现在这里
	bool ServerSetAiming_Valiedate(bool AimingState);//返回false会断开链接
	
	//服务器多播
	UFUNCTION(NetMulticast,Reliable,WithValidation)
	void MultiShooting();//调用用这个
	void MultiShooting_Implementation();//实现在这里	
	bool MultiShooting_Valiedate();//返回false会断开链接

	//服务器多播
	UFUNCTION(NetMulticast,Reliable,WithValidation)
	void MultiSpawnBulletDecal(FVector BulletDecalLocation,FRotator BulletDecalRotation);//调用用这个
	void MultiSpawnBulletDecal_Implementation(FVector BulletDecalLocation,FRotator BulletDecalRotation);//实现在这里	
	bool MultiSpawnBulletDecal_Valiedate(FVector BulletDecalLocation,FRotator BulletDecalRotation);//返回false会断开链接
	
	//服务器多播
	UFUNCTION(NetMulticast,Reliable,WithValidation)
	void MultiReloadAnimation();//调用用这个
	void MultiReloadAnimation_Implementation();//实现在这里	
	bool MultiReloadAnimation_Valiedate();//返回false会断开链接
	
	//联通服务器 Client客户端调用相关的方法
	UFUNCTION(Client,Reliable)
	void ClientEquipFPArmsPrimary();//调用用这个 客户端去装备主武器
	UFUNCTION(Client,Reliable)
	void ClientEquipFPArmsSecondary();//调用用这个 客户端去装备副武器
	
	UFUNCTION(Client,Reliable)
	void ClientFire();

	UFUNCTION(Client,Reliable)
	void ClientUpdateAmmoUI(int32 ClipCurrentAmmo, int32 GunCurrentAmmo);

	UFUNCTION(Client,Reliable)
	void ClientUpdateHealthUI(float NewHealth);

	UFUNCTION(Client,Reliable)
	void ClientReload();
	
	//客户端后坐力方法
	UFUNCTION(Client,Reliable)
	void ClientRecoil();
	
	//客户端开镜
	UFUNCTION(Client,Reliable)
	void ClientAiming();
	
	UFUNCTION(Client,Reliable)
	void ClientEndAiming();

	UFUNCTION(Client,Reliable)
	void ClientDeashShootDeash();

	
#pragma endregion
};
