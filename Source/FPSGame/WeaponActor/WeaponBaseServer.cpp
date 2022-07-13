
#include "WeaponBaseServer.h"
#include "FPSGame/FPSCharacter/FPSCharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


AWeaponBaseServer::AWeaponBaseServer()
{
	PrimaryActorTick.bCanEverTick = true;

	//初始化
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;

	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SphereCollision->SetupAttachment(RootComponent);

	//会掉在地上 需要物理碰撞QueryAndPhysics查询和物理
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);

	WeaponMesh->SetOwnerNoSee(true);
	//开启重力 能掉在地上
	WeaponMesh->SetEnableGravity(true);
	//开启物理模拟 
	WeaponMesh->SetSimulatePhysics(true);

	//当有人进来的时候
	//需要加一个事件进去
	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AWeaponBaseServer::OnOtherBeginOverLap);
	
	//服务器生成后 客户端会生成一份 复制出来 Set会慢 bReplicates快
	bReplicates = true;
	
}


//碰撞事件
void AWeaponBaseServer::OnOtherBeginOverLap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//如果转换成功碰的东西就是玩家
	AFPSCharacterBase* FPSCharacter = Cast<AFPSCharacterBase>(OtherActor);
	if (FPSCharacter)
	{
		//玩家和主武器重叠时候 先进行自我处理
		EquipWeapon();
		//如果有很多手枪可以用switch 根据枪的类型调用不同的方法
		if (KindOfWeapon == EWeaponType::DesertEagle)	//如果是沙鹰-副武器
		{
			FPSCharacter->EquipSecondary(this);//副武器
		}
		else
		{
			//玩家逻辑
			FPSCharacter->EquipPrimary(this);//主武器
		}
	
	}
}

//被拾取的时候自己设置碰撞相关属性
void AWeaponBaseServer::EquipWeapon()
{
	//拿到手里关闭重力 能掉在地上
	WeaponMesh->SetEnableGravity(false);
	//拿到手里关闭物理模拟 
	WeaponMesh->SetSimulatePhysics(false);
	//拿到手里关闭不要有碰撞
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//拿到手里关闭不要有碰撞
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

//射击枪口特效
void AWeaponBaseServer::MultiShootingEffect_Implementation()
{
	// 先屏蔽掉谁调用进来的		                        拿到当前客户端默认的这个Pawn
	if (GetOwner() != UGameplayStatics::GetPlayerPawn(GetWorld(),0))
	{

		FName MuzzleFlashSocketName = TEXT("Fire_FX_Slot");
		if (KindOfWeapon == EWeaponType::M4A1)
		{
			MuzzleFlashSocketName = TEXT("MuzzleSocket");
		}
		UGameplayStatics::SpawnEmitterAttached(MuzzleFlash,WeaponMesh,MuzzleFlashSocketName,
		FVector::ZeroVector,FRotator::ZeroRotator,FVector::OneVector,
		EAttachLocation::KeepRelativeOffset,true,EPSCPoolMethod::None,true);

		//　服务器上要在某一个位子播放声音
		UGameplayStatics::PlaySoundAtLocation(GetWorld(),FireSound,GetActorLocation());
		
	}
}

bool AWeaponBaseServer::MultiShootingEffect_Validate()
{
	return true;
}

void AWeaponBaseServer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//子弹变化
	//这个类里面 标识为Replicated 的一些设置 写在这里 COND_None任何时候 服务端变了之后客户端变
	DOREPLIFETIME_CONDITION(AWeaponBaseServer,CurrentClipAmmo,COND_None);
	
	
}


