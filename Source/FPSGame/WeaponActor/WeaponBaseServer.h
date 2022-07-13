#pragma once

#include "CoreMinimal.h"
#include "WeaponBaseClient.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "WeaponBaseServer.generated.h"


//需要一个枚举  来代表这个武器是一个什么类型， 它在服务器上 必须要有一个属性来表示它是什么类型 装备枪的时候会起到作用
UENUM()
enum class EWeaponType : uint8//缩小
{
	//UMETAUMETA(DisplayName = "XXX"),可以在蓝图中被看到
	AK47 UMETA(DisplayName = "AK47"),
	M4A1 UMETA(DisplayName = "M4A1"),
	MP7 UMETA(DisplayName = "MP7"),
	DesertEagle UMETA(DisplayName = "DesertEagle"),
	Sniper UMETA(DisplayName = "Sniper"),
	EEND
	
};


UCLASS()
class FPSGAME_API AWeaponBaseServer : public AActor
{
	GENERATED_BODY()
	
public:
	
	AWeaponBaseServer();

	//枚举变量 可以代表很多武器 可以在被蓝图中编辑 别人可以拿到 
	UPROPERTY(EditAnywhere)
	EWeaponType KindOfWeapon;

	//组件结构定义
	UPROPERTY(EditAnywhere)
	USkeletalMeshComponent* WeaponMesh;

	//还需要一个碰撞体 玩家可以通过触碰拾取枪械 产生碰撞 需要由Server来做
	UPROPERTY(EditAnywhere)
	USphereComponent* SphereCollision;

	//指定一个类类型 动态创建
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TSubclassOf<AWeaponBaseClient> ClientWeaponBaseBPClass;
	

	//参数必须和系统一样 前面名字可以不一样 6个参数
	UFUNCTION()
	void OnOtherBeginOverLap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void EquipWeapon();
	
	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;
	UPROPERTY(EditAnywhere)
	USoundBase* FireSound;

	UPROPERTY(EditAnywhere,Category="Bullet")
	int32 CurrentGunAmmo;//枪体现在剩余多少子弹
	UPROPERTY(EditAnywhere,Replicated,meta =(AllowPrivateAccess = "true"),Category="Bullet")//Replicated标识 如果服务器上改变了 客户端也要自动改变
	int32 CurrentClipAmmo;//弹夹里面还剩多少子弹
	UPROPERTY(EditAnywhere,Category= "Bullet")
	int32 MaxClipAmmo;//弹夹容量
	UPROPERTY(EditAnywhere,Category = "Bullet")
	float ShootingDistance;//子弹射击距离
	UPROPERTY(EditAnywhere,Category = "Bullet")
	UMaterialInterface* BulletDecalMaterial;//子弹弹孔贴花
	//基础伤害
	UPROPERTY(EditAnywhere,Category = "Bullet")
	float BaseDamage;
	
	UPROPERTY(EditAnywhere,Category="TP_BodyAnimation")
	UAnimMontage* ServerTPBodysShootAnimMontage;
	UPROPERTY(EditAnywhere,Category="TP_BodyAnimation")
	UAnimMontage* ServerTPBodysReloadAnimMontage;
	
	//是否可以连发
	UPROPERTY(EditAnywhere,Category="WeaponAuto?")
	bool IsAutoMatic;
	//连发的频率
	UPROPERTY(EditAnywhere,Category="WeaponAuto?")
	float AutoMaticFireRate;
	//曲线图
	UPROPERTY(EditAnywhere,Category="WeaponCurveData")
	UCurveFloat* VerticalRecoilCurve;
	UPROPERTY(EditAnywhere,Category="WeaponCurveData")
	UCurveFloat* HorizontalRecoilCurve;
	//跑动子弹偏移量
	UPROPERTY(EditAnywhere,Category="WeaponCurveData")
	float MovingFireRandomRange;

	//偏移频率
	UPROPERTY(EditAnywhere,Category="SpreadWeaponData")
	float SpreadWeaponCallBackRate;
	UPROPERTY(EditAnywhere,Category="SpreadWeaponData")
	float SpreadWeaponMinIndex;//手枪的后坐力算法 最小偏移
	UPROPERTY(EditAnywhere,Category="SpreadWeaponData")
	float SpreadWeaponMaxIndex;//手枪的后坐力算法 最大偏移;

	//服务器多播
	UFUNCTION(NetMulticast,Reliable,WithValidation)
	void MultiShootingEffect();//调用用这个
	void MultiShootingEffect_Implementation();//实现在这里	
	bool MultiShootingEffect_Valiedate();//返回false会断开链接
	

};
