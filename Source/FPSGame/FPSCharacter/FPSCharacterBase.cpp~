#include "FPSCharacterBase.h"

#include "Blueprint/UserWidget.h"
#include "Components/DecalComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "PhysicalMaterials/PhysicalMaterial.h"


AFPSCharacterBase::AFPSCharacterBase()
{
 	
	PrimaryActorTick.bCanEverTick = true;

	//初期化作成
#pragma region Component // 折叠宏 组件相关
	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
    if (PlayerCamera)
    {
    	//设置到根组件下
	    PlayerCamera->SetupAttachment(RootComponent);
    	//实现摄像机上下旋转 上下抬头
    	PlayerCamera->bUsePawnControlRotation = true;
    }
	FPArmsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPArmsMesh"));
    if (FPArmsMesh)
    {
    	//设置到相机下
    	FPArmsMesh->SetupAttachment(PlayerCamera);
		//自分だけ見える腕
    	FPArmsMesh->SetOnlyOwnerSee(true);
    }
	
	//模型自己看不到
	Mesh->SetOwnerNoSee(true);
	//设置碰撞 仅查询 模型不需要碰撞 碰撞由胶囊提来
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	//设置碰撞类型
	Mesh->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
#pragma endregion
	
}
#pragma region Engine // 折叠宏 引擎相关
void AFPSCharacterBase::DelayBeginPlayCallBack()
{
	//长久持有这个指针
	FPSPlayerController =Cast<AFPSPlayerController> (GetController());
	if (FPSPlayerController)
	{
		FPSPlayerController->CreatePlayerUI();
		
	}
	else
	{
		//0.5秒之后做DelaySpreadWeaponShootCallBack
		FLatentActionInfo ActionInfo;
		ActionInfo.CallbackTarget = this;
		//过一段时间调用DelayPlayArmReloadCallBack
		ActionInfo.ExecutionFunction = TEXT("DelayBeginPlayCallBack");
		ActionInfo.UUID = FMath::Rand();//Delay管理器 可以给一个随机数
		ActionInfo.Linkage = 0;
		//延迟
		UKismetSystemLibrary::Delay(this,10.0f,ActionInfo);
	}
	
}

void AFPSCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	//绑定hit方法	 
	OnTakePointDamage.AddDynamic(this,&AFPSCharacterBase::OnHit);	//OnTakePointDamage.add//被受到伤害的人给这个方法添加回调，当自己被打的时候这个回调方法自动调用
	Health = 100;
	IsFire = false;
	IsReloading = false;
	IsAiming = false;
	

	//初始化动画蓝图
	ClientArmsAnimBP = FPArmsMesh->GetAnimInstance();
	//初始化动画蓝图
	ServerBodysAnimBP = Mesh->GetAnimInstance();
	
	//长久持有这个指针
	FPSPlayerController =Cast<AFPSPlayerController> (GetController());
	if (FPSPlayerController)
	{
		FPSPlayerController->CreatePlayerUI();
		
	}
	else
	{
		//开启计时器
		//0.5秒之后做DelaySpreadWeaponShootCallBack
		FLatentActionInfo ActionInfo;
		ActionInfo.CallbackTarget = this;
		//过一段时间调用DelayPlayArmReloadCallBack
		ActionInfo.ExecutionFunction = TEXT("DelayBeginPlayCallBack");
		ActionInfo.UUID = FMath::Rand();//Delay管理器 可以给一个随机数
		ActionInfo.Linkage = 0;
		//延迟          
		UKismetSystemLibrary::Delay(this,0.5f,ActionInfo);
	}
	
	//初始的武器
	StartWithKindOfWeapon();
	
}

void AFPSCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	// 让客户端知道是否在做这些 进行数据更新
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//子弹变化
	//这个类里面 标识为Replicated 的一些设置 写在这里 COND_None任何时候 服务端变了之后客户端变
	DOREPLIFETIME_CONDITION(AFPSCharacterBase,IsFire,COND_None);
	DOREPLIFETIME_CONDITION(AFPSCharacterBase,IsReloading,COND_None);
	DOREPLIFETIME_CONDITION(AFPSCharacterBase,ActiveWeapon,COND_None);
	DOREPLIFETIME_CONDITION(AFPSCharacterBase,IsAiming,COND_None);
}


void AFPSCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFPSCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
//绑定输入
	//实现前后左右移动
	InputComponent->BindAxis(TEXT("MoveForWorld"),this,&AFPSCharacterBase::MoveForWorld);
	InputComponent->BindAxis(TEXT("MoveRight"),this,&AFPSCharacterBase::MoveRight);
	//现成的方法 摇头Yaw
	InputComponent->BindAxis(TEXT("Turn"),this,&AFPSCharacterBase::AddControllerYawInput);
	//现成的方法 点头Pitch
	InputComponent->BindAxis(TEXT("LookUp"),this,&AFPSCharacterBase::AddControllerPitchInput);
	//实现跳跃
	InputComponent->BindAction(TEXT("Jump"),IE_Pressed,this,&AFPSCharacterBase::JumpAction);
	InputComponent->BindAction(TEXT("Jump"),IE_Released,this,&AFPSCharacterBase::StopJumpAction);
	//Shift减慢速度
	InputComponent->BindAction(TEXT("LowSpeedWalk"),IE_Pressed,this,&AFPSCharacterBase::LowSpeedWalkAction);
	InputComponent->BindAction(TEXT("LowSpeedWalk"),IE_Released,this,&AFPSCharacterBase::NormalSpeedWalkAction);
	//鼠标左键按下开火
	InputComponent->BindAction(TEXT("Fire"),IE_Pressed,this,&AFPSCharacterBase::InputFirePressed);
	InputComponent->BindAction(TEXT("Fire"),IE_Released,this,&AFPSCharacterBase::InputFireReleased);
	//开镜OpenAiming
	InputComponent->BindAction(TEXT("Aiming"),IE_Pressed,this,&AFPSCharacterBase::InputAimingPressed);
	InputComponent->BindAction(TEXT("Aiming"),IE_Released,this,&AFPSCharacterBase::InputAimingReleased);
	//换弹夹
	InputComponent->BindAction(TEXT("ReLoad"),IE_Pressed,this,&AFPSCharacterBase::InputReload);
	
	
}
#pragma endregion

#pragma region NetWorking // 折叠宏 网络相关
void AFPSCharacterBase::ServerLowSpeedWalkAction_Implementation()
{
	//在服务器上
	CharacterMovement->MaxWalkSpeed = 300;
}


bool AFPSCharacterBase::ServerLowSpeedWalkAction_Validate()
{
	//不断开链接
	return true;
}

void AFPSCharacterBase::ServerNormalSpeedWalkAction_Implementation()
{
	//在服务器上
	CharacterMovement->MaxWalkSpeed = 600;
}
//服务器开火逻辑
void AFPSCharacterBase::ServerFireRifleWeapon_Implementation(FVector CameraLocktion, FRotator CameraRotation,
	bool IsMoving)
{
	if (ServerPrimryWeapon)
	{
		//多播(必须在服务器调才能生效 谁调谁多播 )
		ServerPrimryWeapon->MultiShootingEffect();
		ServerPrimryWeapon->CurrentClipAmmo -= 1;	// 开枪后子弹减1
		// 多播(播放身体射击动画蒙太奇 会在所有客户端执行)
		MultiShooting();
		// 客户端更新子弹UI
		ClientUpdateAmmoUI(ServerPrimryWeapon->CurrentClipAmmo,ServerPrimryWeapon->CurrentGunAmmo);
	}
	//是在开火
	IsFire = true;
	//射线检测方法调用
	RilfeLineTrace(CameraLocktion,CameraRotation,IsMoving);
	//UKismetSystemLibrary::PrintString(this,FString::Printf(TEXT("ServerPrimryWeapon->ClipCurrentAmmo:%d"),ServerPrimryWeapon->ClipCurrentAmmo));
}

void AFPSCharacterBase::ServerFirePistolWeapon_Implementation(FVector CameraLocktion, FRotator CameraRotion,
	bool IsMoving)
{
	
	if (ServerSecondaryWeapon)
	{
		//0.5秒之后做DelaySpreadWeaponShootCallBack
		FLatentActionInfo ActionInfo;
		ActionInfo.CallbackTarget = this;
		//过一段时间调用DelayPlayArmReloadCallBack
		ActionInfo.ExecutionFunction = TEXT("DelaySpreadWeaponShootCallBack");
		ActionInfo.UUID = FMath::Rand();//Delay管理器 可以给一个随机数
		ActionInfo.Linkage = 0;
		//延迟
		UKismetSystemLibrary::Delay(this,ServerSecondaryWeapon->SpreadWeaponCallBackRate,ActionInfo);
		
		//多播(必须在服务器调才能生效 谁调谁多播 )
		ServerSecondaryWeapon->MultiShootingEffect();
		ServerSecondaryWeapon->CurrentClipAmmo -= 1;	// 开枪后子弹减1
		// 多播(播放身体射击动画蒙太奇 会在所有客户端执行)
		MultiShooting();
		// 客户端更新子弹UI
		ClientUpdateAmmoUI(ServerSecondaryWeapon->CurrentClipAmmo,ServerSecondaryWeapon->CurrentGunAmmo);
	}
	//是在开火
	IsFire = true;
	//射线检测方法调用
	PistolLineTrace(CameraLocktion,CameraRotion,IsMoving);
}

void AFPSCharacterBase::ServerFireSniperWeapon_Implementation(FVector CameraLocation, FRotator CameraRotation,
	bool IsMoving)
{
	if (ServerPrimryWeapon)
	{
		
		//多播(必须在服务器调才能生效 谁调谁多播 )
		ServerPrimryWeapon->MultiShootingEffect();
		ServerPrimryWeapon->CurrentClipAmmo -= 1;	// 开枪后子弹减1
		// 多播(播放身体射击动画蒙太奇 会在所有客户端执行)
		MultiShooting();
		// 客户端更新子弹UI
		ClientUpdateAmmoUI(ServerPrimryWeapon->CurrentClipAmmo,ServerPrimryWeapon->CurrentGunAmmo);
	}
	//狙击枪所有动画播放完了 才能进入下一次射击
	if (ClientPrimaryWeapon)
	{
		//开启计时器
		//0.5秒之后做DelaySpreadWeaponShootCallBack
		FLatentActionInfo ActionInfo;
		ActionInfo.CallbackTarget = this;
		//过一段时间调用DelayPlayArmReloadCallBack
		ActionInfo.ExecutionFunction = TEXT("DelaySniperShootCallBack");
		ActionInfo.UUID = FMath::Rand();//Delay管理器 可以给一个随机数
		ActionInfo.Linkage = 0;
		//延迟                                                    //动画播放结束	
		UKismetSystemLibrary::Delay(this,ClientPrimaryWeapon->ClientArmsFireAnimMontage->GetPlayLength(),
			ActionInfo);
	}
	//是在开火
	IsFire = true;
	//射线检测方法调用
	SniperLineTrace(CameraLocation,CameraRotation,IsMoving);
	
}

bool AFPSCharacterBase::ServerFireSniperWeapon_Validate(FVector CameraLocktion, FRotator CameraRotion, bool IsMoving)
{
	return true;
}

bool AFPSCharacterBase::ServerFirePistolWeapon_Validate(FVector CameraLocktion, FRotator CameraRotion, bool IsMoving)
{

	return true;
}


void AFPSCharacterBase::ServerReloadPrimary_Implementation()
{
	
	//UKismetSystemLibrary::PrintString(this,FString::Printf(TEXT("ServerReloadPrimary_Implementation")));
	if (ServerPrimryWeapon)
	{
		if (ServerPrimryWeapon->CurrentGunAmmo > 0 && ServerPrimryWeapon->CurrentClipAmmo < ServerPrimryWeapon->MaxClipAmmo)
		{
			//客户端手臂播放动画(done)，服务器身体播放动画 数据更新，UI更改
			ClientReload();
			MultiReloadAnimation();
			IsReloading = true;
			if (ClientPrimaryWeapon)
			{
				FLatentActionInfo ActionInfo;
				ActionInfo.CallbackTarget = this;
				//过一段时间调用DelayPlayArmReloadCallBack
				ActionInfo.ExecutionFunction = TEXT("DelayPlayArmReloadCallBack");
				ActionInfo.UUID = FMath::Rand();//Delay管理器 可以给一个随机数
				ActionInfo.Linkage = 0;
				//延迟
				UKismetSystemLibrary::Delay(this,ClientPrimaryWeapon->ClientArmsReloadAnimMontage->GetPlayLength(),ActionInfo);
				
			}
			
			
		}
		
	}
	
}
bool AFPSCharacterBase::ServerReloadPrimary_Validate()
{
	return true;
}



void AFPSCharacterBase::ServerStopFire_Implementation()
{
	IsFire = false;
}

void AFPSCharacterBase::ServerReloadSecondary_Implementation()
{
		
	//UKismetSystemLibrary::PrintString(this,FString::Printf(TEXT("ServerReloadPrimary_Implementation")));
	if (ServerSecondaryWeapon)
	{
		if (ServerSecondaryWeapon->CurrentGunAmmo > 0 && ServerSecondaryWeapon->CurrentClipAmmo < ServerSecondaryWeapon->MaxClipAmmo)
		{
			//客户端手臂播放动画(done)，服务器身体播放动画 数据更新，UI更改
			ClientReload();
			MultiReloadAnimation();
			IsReloading = true;
			if (ClientSecondaryWeapon)
			{
				FLatentActionInfo ActionInfo;
				ActionInfo.CallbackTarget = this;
				//过一段时间调用DelayPlayArmReloadCallBack
				ActionInfo.ExecutionFunction = TEXT("DelayPlayArmReloadCallBack");
				ActionInfo.UUID = FMath::Rand();//Delay管理器 可以给一个随机数
				ActionInfo.Linkage = 0;
				//延迟
				UKismetSystemLibrary::Delay(this,ClientSecondaryWeapon->ClientArmsReloadAnimMontage->GetPlayLength(),ActionInfo);
				
			}
			
			
		}
		
	}
	
	
}

bool AFPSCharacterBase::ServerReloadSecondary_Validate()
{
	return true;
}

bool AFPSCharacterBase::ServerStopFire_Validate()
{
	return true;
}

void AFPSCharacterBase::MultiShooting_Implementation()
{
	AWeaponBaseServer* CurrentServerWeapon = GetCurrentServerTPBodysWeaponActor();
	if (ServerBodysAnimBP)
	{
		if (CurrentServerWeapon)
		{
			//找到身上带的动画蒙太奇
			ServerBodysAnimBP->Montage_Play(CurrentServerWeapon->ServerTPBodysShootAnimMontage);	
		}
		
	}
	
}

void AFPSCharacterBase::MultiSpawnBulletDecal_Implementation(FVector BulletDecalLocation,FRotator BulletDecalRotation)
{
	//共用
	AWeaponBaseServer* CurrentServerWeapon = GetCurrentServerTPBodysWeaponActor();

	if (CurrentServerWeapon)
	{
		
		UDecalComponent* Decal = UGameplayStatics::SpawnDecalAtLocation(GetWorld(),CurrentServerWeapon->BulletDecalMaterial,FVector(8,8,8),
			BulletDecalLocation,BulletDecalRotation,10);
		if (Decal)
		{
			Decal->SetFadeScreenSize(0.001f);
		}
		
	}

}

void AFPSCharacterBase::MultiReloadAnimation_Implementation()
{
	//Server先去拿到枪
	AWeaponBaseServer* CurrentServerWeapon = GetCurrentServerTPBodysWeaponActor();
	//看看身体上的动画蓝图在不在
	if (ServerBodysAnimBP)
	{
		 //再看枪在不在
		if (CurrentServerWeapon)
		{
			//找到身上带的动画蒙太奇
			ServerBodysAnimBP->Montage_Play(CurrentServerWeapon->ServerTPBodysReloadAnimMontage);	
		}
		
	}
	
}

void AFPSCharacterBase::ClientDeashShootDeash_Implementation()
{
	AWeaponBaseClient* CurrentClientBase = GetCurrentClintFPArmsWeaponActor();
	
	if (CurrentClientBase)
	{
		CurrentClientBase->Destroy();
	}
	
}


void AFPSCharacterBase::ClientAiming_Implementation()
{
	if (FPArmsMesh)
	{
		FPArmsMesh->SetHiddenInGame(true);		
	}
	
	if (ClientPrimaryWeapon)
	{
		//瞄准时不可见
		ClientPrimaryWeapon->SetActorHiddenInGame(true);

		if (PlayerCamera)
		{
			//设置能看到的距离
			PlayerCamera->SetFieldOfView(ClientPrimaryWeapon->FieldAimingView);	
		}
		
	}
	//创建出来
	WidgetScope = CreateWidget<UUserWidget>(GetWorld(),SniperScopeBPClass);
	WidgetScope->AddToViewport();//加入屏幕
}

void AFPSCharacterBase::ClientEndAiming_Implementation()
{
	if (FPArmsMesh)
	{
		FPArmsMesh->SetHiddenInGame(false);		
	}
	
	if (ClientPrimaryWeapon)
	{
		//瞄准时不可见
		ClientPrimaryWeapon->SetActorHiddenInGame(false);

		if (PlayerCamera)
		{
			//设置能看到的距离
			PlayerCamera->SetFieldOfView(90);	
		}
		
	}
	if (WidgetScope)
	{
		//删除
		WidgetScope->RemoveFromParent();		
	}
}



void AFPSCharacterBase::ServerSetAiming_Implementation(bool AimingState)
{
	IsAiming = AimingState;
}

bool AFPSCharacterBase::ServerSetAiming_Validate(bool AimingState)
{
	return true;
}

//副武器
void AFPSCharacterBase::ClientEquipFPArmsSecondary_Implementation()
{
	//我们要去通过Server去拿到 ClientWeaponBaseBPClass
	if (ServerSecondaryWeapon)
	{
		if (ClientSecondaryWeapon)
		{
			//客户端有了 什么都不做
			return;
		}
		else
		{
			FActorSpawnParameters SpawnInfo;//SpawnActor所需要的信息
			SpawnInfo.Owner = this;//拥有这是谁
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;//SpawnActor的规则 不管如何都生成

			// 动态创建出来一个对象出来
			ClientSecondaryWeapon = GetWorld()->SpawnActor<AWeaponBaseClient>(ServerSecondaryWeapon->ClientWeaponBaseBPClass,
				GetActorTransform(),
				SpawnInfo);	

			FName WeaponSocketName = TEXT("WeaponSocket");
			// 不同武器的手臂插槽不一样 2
			ClientSecondaryWeapon->K2_AttachToComponent(FPArmsMesh,WeaponSocketName,EAttachmentRule::SnapToTarget,
				EAttachmentRule::SnapToTarget,
				EAttachmentRule::SnapToTarget,
				true);
			
			// 一开始就初始化弹夹
			ClientUpdateAmmoUI(ServerSecondaryWeapon->CurrentClipAmmo,ServerSecondaryWeapon->CurrentGunAmmo);
			// 改变手臂动画
			if (ClientSecondaryWeapon)
			{
				UpdateFPArmsBlendPose(ClientSecondaryWeapon->FPArmsBlendPose);	
			}
			
		}
	
	}
	
}



bool AFPSCharacterBase::MultiReloadAnimation_Validate()
{
	return true;
}


void AFPSCharacterBase::ClientReload_Implementation()
{
	//会根据我们现在的weapon 拿到武器
	AWeaponBaseClient* CurrentClintWeapon = GetCurrentClintFPArmsWeaponActor();
	if (CurrentClintWeapon)
	{
		UAnimMontage* ClientArmsReloadMontage = CurrentClintWeapon->ClientArmsReloadAnimMontage;
		ClientArmsAnimBP->Montage_Play(ClientArmsReloadMontage);
		CurrentClintWeapon->PlayReloadAnimation();
	}

}


void AFPSCharacterBase::ClientRecoil_Implementation()
{
	UCurveFloat* VercicalRecoilCurve = nullptr;//拿到曲线图
	UCurveFloat* HorizontalRecoilCurve = nullptr;
	if (ServerPrimryWeapon)
	{
		VercicalRecoilCurve = ServerPrimryWeapon->VerticalRecoilCurve;
		HorizontalRecoilCurve = ServerPrimryWeapon->HorizontalRecoilCurve;
	}
	
	RecoilXCoordPerShoot += 0.1f;

	if (VercicalRecoilCurve)
	{
		NewVerticalRecoilAmount = VercicalRecoilCurve->GetFloatValue(RecoilXCoordPerShoot);//每次把新的点拿到
		
	}
	if (HorizontalRecoilCurve)
	{
		NewHorizontalRecoilAmount = HorizontalRecoilCurve->GetFloatValue(RecoilXCoordPerShoot);//每次把新的点拿到
		
	}
	
	VerticalRecoilAmount = NewVerticalRecoilAmount - OldVerticalRecoilAmount;//拿到插值
	HorizontalRecoilAmount = NewHorizontalRecoilAmount - OldHorizontalRecoilAmount;//拿到插值
	if (FPSPlayerController)
	{
		FRotator ControllerRotator = FPSPlayerController->GetControlRotation();
		FPSPlayerController->SetControlRotation(FRotator(ControllerRotator.Pitch + VerticalRecoilAmount,
		ControllerRotator.Yaw + HorizontalRecoilAmount,
		ControllerRotator.Roll));//插值应用Pitch上
		
	}
	
	OldVerticalRecoilAmount = NewVerticalRecoilAmount;
	OldHorizontalRecoilAmount = NewHorizontalRecoilAmount;
}



//在客户端更新弹夹UI
void AFPSCharacterBase::ClientUpdateHealthUI_Implementation(float NewHealth)
{
	if (FPSPlayerController)
	{
		FPSPlayerController->UpdateHeathUI(NewHealth);
		
	}
	
}



bool AFPSCharacterBase::MultiSpawnBulletDecal_Validate(FVector BulletDecalLocation,FRotator BulletDecalRotation)
{
	return true;
}


bool AFPSCharacterBase::MultiShooting_Validate()
{
	return true;
}

void AFPSCharacterBase::ClientUpdateAmmoUI_Implementation(int32 ClipCurrentAmmo, int32 GunCurrentAmmo)
{
	if (FPSPlayerController)
	{
		FPSPlayerController->UpdateAmmoUI(ClipCurrentAmmo,GunCurrentAmmo);
		
	}
	
}


bool AFPSCharacterBase::ServerFireRifleWeapon_Validate(FVector CameraLocktion, FRotator CameraRotation, bool IsMoving)
{
	return true;
}

//客户端开火逻辑
void AFPSCharacterBase::ClientFire_Implementation()
{
	//枪体播放动画实现
	AWeaponBaseClient* CurrentClintWeapon = GetCurrentClintFPArmsWeaponActor();
	//如果能拿到我们调用PlayShootAnimation
	if (CurrentClintWeapon)
	{
		//播放枪的动画
		CurrentClintWeapon->PlayShootAnimation();
		//手臂播放动画
		UAnimMontage* ClientArmsFireMontage = CurrentClintWeapon->ClientArmsFireAnimMontage;
		ClientArmsAnimBP->Montage_SetPlayRate(ClientArmsFireMontage,1);	//一倍速
		ClientArmsAnimBP->Montage_Play(ClientArmsFireMontage);
		//播放射击声音 特效
		CurrentClintWeapon->DisplayWeaponEffect();

		
		if (FPSPlayerController)
		{
			//应用屏幕抖动
			FPSPlayerController->PlayerCameraShake(CurrentClintWeapon->CameraShakeClass);//ok
			//播放十字线扩散动画
			FPSPlayerController->DoCrosshairRecoil();//ok
		}
	}

}



void AFPSCharacterBase::ClientEquipFPArmsPrimary_Implementation()
{
	//我们要去通过Server去拿到 ClientWeaponBaseBPClass
	if (ServerPrimryWeapon)
	{
		if (ClientPrimaryWeapon)
		{
			//客户端有了 什么都不做
			return;
		}
		else
		{
			FActorSpawnParameters SpawnInfo;//SpawnActor所需要的信息
			SpawnInfo.Owner = this;//拥有这是谁
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;//SpawnActor的规则 不管如何都生成

			// 动态创建出来一个对象出来
			ClientPrimaryWeapon = GetWorld()->SpawnActor<AWeaponBaseClient>(	ServerPrimryWeapon->ClientWeaponBaseBPClass,
				GetActorTransform(),
				SpawnInfo);	

			FName WeaponSocketName = TEXT("WeaponSocket");
			if (ActiveWeapon == EWeaponType::M4A1)
			{
				WeaponSocketName = TEXT("M4A1_Socket");
			
			}
			if (ActiveWeapon == EWeaponType::Sniper)
			{
				WeaponSocketName = TEXT("AWP_Socket");
				
			}
			// 不同武器的手臂插槽不一样
			ClientPrimaryWeapon->K2_AttachToComponent(FPArmsMesh,WeaponSocketName,EAttachmentRule::SnapToTarget,
				EAttachmentRule::SnapToTarget,
				EAttachmentRule::SnapToTarget,
				true);
			
			// 一开始就初始化弹夹
			ClientUpdateAmmoUI(ServerPrimryWeapon->CurrentClipAmmo,ServerPrimryWeapon->CurrentGunAmmo);
			// 改变手臂动画
			if (ClientPrimaryWeapon)
			{
				UpdateFPArmsBlendPose(ClientPrimaryWeapon->FPArmsBlendPose);	
			}
			
		}
	
	}
	
}

bool AFPSCharacterBase::ServerNormalSpeedWalkAction_Validate()
{
	//不断开链接
	return true;
}
#pragma endregion 

#pragma region InputEvent // 折叠宏 输入相关
void AFPSCharacterBase::MoveForWorld(float AxisValue)
{
	//向前 后
	AddMovementInput(GetActorForwardVector(),AxisValue,false);
}

void AFPSCharacterBase::MoveRight(float AxisValue)
{
	//向左 右
	AddMovementInput(GetActorRightVector(),AxisValue,false);
}

void AFPSCharacterBase::JumpAction()
{
	//现成方法
	Jump();
}

void AFPSCharacterBase::StopJumpAction()
{
	//现成方法
	StopJumping();
}

void AFPSCharacterBase::LowSpeedWalkAction()
{
	//要拿到movement组件
	//修改的是客户端的速度
	CharacterMovement->MaxWalkSpeed = 300;

	//调用服务器上的
	ServerLowSpeedWalkAction();
	
}

void AFPSCharacterBase::NormalSpeedWalkAction()
{
	//要拿到movement组件
	//修改的是客户端的速度
	CharacterMovement->MaxWalkSpeed = 600;
	//调用服务器上的
	ServerNormalSpeedWalkAction();
}

void AFPSCharacterBase::InputFirePressed()
{
	//更具武器类型来调用那种武器的射击方法
	switch (ActiveWeapon)
	{
		case EWeaponType::AK47:
			{
				FireWeaponPrimary();
			}
			break;
		case EWeaponType::M4A1:
			{
				FireWeaponPrimary();
			}
			break;
		case EWeaponType::MP7:
			{
				FireWeaponPrimary();
			}
			break;
		case EWeaponType::DesertEagle:
			{
				FireWeaponSecondary();
			}
			break;
		case EWeaponType::Sniper:
			{
				FireSniperPrimary();
			}
			break;
		
	}
	
}

void AFPSCharacterBase::InputFireReleased()
{
	//更具武器类型来调用那种武器的射击方法
	switch (ActiveWeapon)
	{
		case EWeaponType::AK47:
			{
				StopFirePrimary();
			}
		break;
		case EWeaponType::M4A1:
			{
				StopFirePrimary();
			}
		break;
		case EWeaponType::MP7:
			{
				StopFirePrimary();
			}
		break;
		case EWeaponType::DesertEagle:
			{
				StopFireSecondary();
			}
			break;
		case EWeaponType::Sniper:
			{
				
			}
			break;
	
	}
	
}

void AFPSCharacterBase::InputReload()
{
	if (!IsReloading)
	{
		if (!IsFire)
		{
			switch (ActiveWeapon)
			{
			case EWeaponType::AK47:
				{
					
					ServerReloadPrimary();
				}
				break;
			case EWeaponType::M4A1:
				{
					
					ServerReloadPrimary();
				}
				break;
			case EWeaponType::MP7:
				{
					
					ServerReloadPrimary();
				}
				break;
			case EWeaponType::DesertEagle:
				{
					
					ServerReloadSecondary();
				}
				break;
			case EWeaponType::Sniper:
				{
					
					ServerReloadPrimary();
				}
				break;
			}	
		}
	}
	
	
}

void AFPSCharacterBase::InputAimingPressed()
{
	//贴图面准镜的UI 关闭枪体可见性 摄像头距离拉远 需要客户端RPC 和 服务器RPC
	//更改Aiming 属性 服务器RPC
	if (ActiveWeapon == EWeaponType::Sniper)//当前武器是狙击枪才做
	{
		ServerSetAiming(true);
		ClientAiming();
	}
}

void AFPSCharacterBase::InputAimingReleased()
{
	//贴图面准镜的UI删除 开启枪体可见性 摄像头距离恢复
	if (ActiveWeapon == EWeaponType::Sniper)//当前武器是狙击枪才做
	{
		ServerSetAiming(false);
		ClientEndAiming();
	}
}


#pragma endregion 

#pragma region Weapon // 折叠宏 装备武器相关

//服务器上运行
void AFPSCharacterBase::DeathShootDeath(AActor* DamageActor)
{
	//死亡先删除服务器上武器
	AWeaponBaseClient* CurrentClientBase = GetCurrentClintFPArmsWeaponActor();
	AWeaponBaseServer* CurrentServerBase = GetCurrentServerTPBodysWeaponActor();
	if (CurrentClientBase)
	{
		CurrentClientBase->Destroy();
	}
	if (CurrentServerBase)
	{
		CurrentServerBase->Destroy();
	}
	//然后删除客户端上武器
	ClientDeashShootDeash();

	AFPSPlayerController* PlayerController = Cast<AFPSPlayerController>(GetController());
	if (PlayerController)
	{							//伤害我们的人
		PlayerController->Death(DamageActor);
	}
	
}

void AFPSCharacterBase::EquipPrimary(AWeaponBaseServer* WeaponBaseServer)
{
	//判断是否有主武器 没武器才装备
	if (ServerPrimryWeapon)
	{
		return;
	}
	else
	{
		//拿到主武器 存到自己的身上
		ServerPrimryWeapon = WeaponBaseServer;
		//属于自己
		ServerPrimryWeapon->SetOwner(this);
		//添加到第三人称的body上
		ServerPrimryWeapon->K2_AttachToComponent(Mesh,TEXT("Weapon_Rifle"),EAttachmentRule::SnapToTarget,
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::SnapToTarget,
			true);

		ActiveWeapon = ServerPrimryWeapon->KindOfWeapon;//服务器上改
		//通知到相关的客户端去生成
		ClientEquipFPArmsPrimary();
	}
}

void AFPSCharacterBase::EquipSecondary(AWeaponBaseServer* WeaponBaseServer)
{
	//判断是否存在副武器 没武器才装备
	if (ServerSecondaryWeapon)
	{
		return;
	}
	else
	{
		//拿到副武器存到自己的身上
		ServerSecondaryWeapon = WeaponBaseServer;
		//属于自己 Chareator
		ServerSecondaryWeapon->SetOwner(this);
		//添加到第三人称的body上                                   //有第三人称的手枪动画可以替换
		ServerSecondaryWeapon->K2_AttachToComponent(Mesh,TEXT("Weapon_Rifle"),EAttachmentRule::SnapToTarget,
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::SnapToTarget,
			true);

		//
		ActiveWeapon = ServerSecondaryWeapon->KindOfWeapon;//服务器上改
		//通知到相关的客户端去生成
		ClientEquipFPArmsSecondary();

	}
	
}

//生成角色身上的武器
void AFPSCharacterBase::StartWithKindOfWeapon()
{
	//HasAuthority 判断当前的代码是否对人物有主动权
	//如果有主控权那么是在服务器上 不是100%
	if (HasAuthority())
	{
		// 在服务器上拿什么枪
		// 只会在服务器上调用　买武器
		// 随机武器   父类uint8转EWeapon子类不安全 所哟要用static_cast强转 不能用cast
		PurChaseWeapon(static_cast<EWeaponType>(UKismetMathLibrary::RandomFloatInRange(0,static_cast<int8>(EWeaponType::EEND) - 1)));
		
	}
	
}

void AFPSCharacterBase::PurChaseWeapon(EWeaponType WeaponType)
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = this;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	switch (WeaponType)
	{
	case EWeaponType::AK47:
		{
			//动态拿到AK47 server类
			UClass* BlueprintVer = StaticLoadClass(AWeaponBaseServer::StaticClass(),
				nullptr,
				TEXT("Blueprint'/Game/Blueprints/Weapon/AK47/ServerBP_AK47.ServerBP_AK47_C'"));
			//拿到之后创建出来
			AWeaponBaseServer* ServerWeapon = GetWorld()->SpawnActor<AWeaponBaseServer>(BlueprintVer,
			GetActorTransform(),
			SpawnInfo);
		
			//主动搞了个碰撞
			ServerWeapon->EquipWeapon();
			ActiveWeapon = EWeaponType::AK47;//服务器上改
			EquipPrimary(ServerWeapon);
			
		}
		break;
	case  EWeaponType::M4A1:	
		{
			//动态拿到M4A1 server类
			UClass* BlueprintVer = StaticLoadClass(AWeaponBaseServer::StaticClass(),
				nullptr,
				TEXT("Blueprint'/Game/Blueprints/Weapon/M4A1/ServerBP_M4A1.ServerBP_M4A1_C'"));
			//拿到之后创建出来
			AWeaponBaseServer* ServerWeapon = GetWorld()->SpawnActor<AWeaponBaseServer>(BlueprintVer,
			GetActorTransform(),
			SpawnInfo);
			
			//主动搞了个碰撞
			ServerWeapon->EquipWeapon();
			ActiveWeapon = EWeaponType::M4A1;//服务器上改
			EquipPrimary(ServerWeapon);
			
		}
		break;
	case  EWeaponType::MP7:	
		{
			//动态拿到MP7 server类
			UClass* BlueprintVer = StaticLoadClass(AWeaponBaseServer::StaticClass(),
				nullptr,
				TEXT("Blueprint'/Game/Blueprints/Weapon/MP7/ServerBP_MP7.ServerBP_MP7_C'"));
			//拿到之后创建出来
			AWeaponBaseServer* ServerWeapon = GetWorld()->SpawnActor<AWeaponBaseServer>(BlueprintVer,
			GetActorTransform(),
			SpawnInfo);
			
			//主动搞了个碰撞
			ServerWeapon->EquipWeapon();
			EquipPrimary(ServerWeapon);
			
		}
		break;
	case  EWeaponType::DesertEagle:	
		{
			//动态拿到MDesertEagle server类 1
			UClass* BlueprintVer = StaticLoadClass(AWeaponBaseServer::StaticClass(),
				nullptr,
				TEXT("Blueprint'/Game/Blueprints/Weapon/DesertEagle/ServerBP_DesertEagle.ServerBP_DesertEagle_C'"));
			//拿到之后创建出来 生成这个类
			AWeaponBaseServer* ServerWeapon = GetWorld()->SpawnActor<AWeaponBaseServer>(BlueprintVer,
			GetActorTransform(),
			SpawnInfo);
			
			//主动搞了个碰撞
			ServerWeapon->EquipWeapon();
			//调用副武器
			EquipSecondary(ServerWeapon);
			
		}
		break;
	case  EWeaponType::Sniper:	
		{
			//动态拿到Sniper server类 1
			UClass* BlueprintVer = StaticLoadClass(AWeaponBaseServer::StaticClass(),
				nullptr,
				TEXT("Blueprint'/Game/Blueprints/Weapon/AWP/ServerBP_AWP.ServerBP_AWP_C'"));
			//拿到之后创建出来 生成这个类
			AWeaponBaseServer* ServerWeapon = GetWorld()->SpawnActor<AWeaponBaseServer>(BlueprintVer,
			GetActorTransform(),
			SpawnInfo);
			
			//主动搞了个碰撞
			ServerWeapon->EquipWeapon();
			//调用副武器
			EquipPrimary(ServerWeapon);
			
		}
		break;
	default:
		{
			
		}
		
	}
}

//拿到这个方法我们现在使用武器的客户端的类型的指针    共用的方法
AWeaponBaseClient* AFPSCharacterBase::GetCurrentClintFPArmsWeaponActor()
{
	switch (ActiveWeapon)
	{

	case EWeaponType::AK47:
		{
			return ClientPrimaryWeapon;	
		}
		break;
	case EWeaponType::M4A1:
		{
			return ClientPrimaryWeapon;	
		}
		break;
	case EWeaponType::MP7:
		{
			return ClientPrimaryWeapon;	
		}
		break;
	case EWeaponType::DesertEagle:
		{
			return ClientSecondaryWeapon;	
		}
		break;
	case EWeaponType::Sniper:
		{
			return ClientPrimaryWeapon;	
		}
		break;
	}

	//所有情况不在的时候返回空
	return nullptr; 
	
}
//共用的方法
AWeaponBaseServer* AFPSCharacterBase::GetCurrentServerTPBodysWeaponActor()
{
	switch (ActiveWeapon)
	{

	case EWeaponType::AK47:
		{
			return ServerPrimryWeapon;	
		}
		break;
	case EWeaponType::M4A1:
		{
			return ServerPrimryWeapon;	
		}
		break;
	case EWeaponType::MP7:
		{
			return ServerPrimryWeapon;	
		}
		break;
	case EWeaponType::DesertEagle:
		{
			return ServerSecondaryWeapon;	
		}
		break;
	case EWeaponType::Sniper:
		{
			return ServerPrimryWeapon;	
		}
		break;
		
	}

	//所有情况不在的时候返回空
	return nullptr; 
	
}

#pragma endregion 

#pragma region Fire // 折叠宏 射击相关

void AFPSCharacterBase::AutomaticFire()
{
	//子弹是否足够才能开火
	if (ServerPrimryWeapon->CurrentClipAmmo > 0)//弹夹里面有多少子弹 大于0才能开枪
	{
		//服务器端执行(枪口闪光粒子效果(能被其他玩家看到听到)(done),播放射击声音(能被其他玩家看到听到)(done),减少弹药(done),创建UI子(done),弹更新(done),射线检测(3种步枪，手枪，狙击枪)，伤害应用，弹孔生成)
		if (UKismetMathLibrary::VSize(GetVelocity()) > 0.1f)//VSize算出一个准确的长度 根号下里面x2+Y2+Z2
		{

			ServerFireRifleWeapon(PlayerCamera->GetComponentLocation(),PlayerCamera->GetComponentRotation(),true);
			
		}
		else
		{
			ServerFireRifleWeapon(PlayerCamera->GetComponentLocation(),PlayerCamera->GetComponentRotation(),false);
		}
		//客户端执行(墙体播放动画(done)，手臂播放动画(done)，播放射击声音(done)，应用屏幕抖动(done)，应用后座力，枪口闪光粒子效果(done))
		//客户端 (十字线瞄准UI(done) 初始化UI(done)， 播放十字线瞄准扩散动画(done))
		ClientFire();
		ClientRecoil();
		
 	}
	else
	{
		StopFirePrimary();
	}
	
}

void AFPSCharacterBase::ResetRecoil()
{
	 NewVerticalRecoilAmount = 0;
	 OldVerticalRecoilAmount = 0;
	 VerticalRecoilAmount = 0;
	 RecoilXCoordPerShoot = 0;

	 NewHorizontalRecoilAmount = 0;//新的点
	 OldHorizontalRecoilAmount = 0;//上一个点
	 HorizontalRecoilAmount = 0;//减的值
}

void AFPSCharacterBase::FireWeaponPrimary()
{
	//UE_LOG(LogTemp,Warning,TEXT("void AFPSCharacterBase::FireWeaponPrimary()"));

	//子弹是否足够 和 不在换弹的时候 才能射击 
	if (ServerPrimryWeapon->CurrentClipAmmo > 0 && !IsReloading)//弹夹里面有多少子弹 大于0才能开枪
	{
		//服务器端执行(枪口闪光粒子效果(能被其他玩家看到听到)(done),播放射击声音(能被其他玩家看到听到)(done),减少弹药(done),创建UI子(done),弹更新(done),射线检测(3种步枪，手枪，狙击枪)，伤害应用，弹孔生成) 
		if (UKismetMathLibrary::VSize(GetVelocity()) > 0.1f)//VSize算出一个准确的长度 根号下里面x2+Y2+Z2
		{
			ServerFireRifleWeapon(PlayerCamera->GetComponentLocation(),PlayerCamera->GetComponentRotation(),true);
	
		}
		else
		{
			ServerFireRifleWeapon(PlayerCamera->GetComponentLocation(),PlayerCamera->GetComponentRotation(),false);
		}
		
		//客户端执行(墙体播放动画(done)，手臂播放动画(done)，播放射击声音(done)，应用屏幕抖动(done)，应用后座力，枪口闪光粒子效果(done))
		//客户端 (十字线瞄准UI(done) 初始化UI(done)， 播放十字线瞄准扩散动画(done))
		ClientFire();
		ClientRecoil();
		
		//开启计时器 每隔固定时间重新射击
		if (ServerPrimryWeapon->IsAutoMatic)
		{
		
			GetWorldTimerManager().SetTimer(AutomaticFireTimerHandle,this,&AFPSCharacterBase::AutomaticFire,
				ServerPrimryWeapon->AutoMaticFireRate,
				true);
			//连续射击系统开发
			//UKismetSystemLibrary::PrintString(this,FString::Printf(TEXT("ServerPrimryWeapon->ClipCurrentAmmo:%d"),ServerPrimryWeapon->ClipCurrentAmmo));
			
		}
		
	}

}

void AFPSCharacterBase::StopFirePrimary()
{
	//归0
	//关闭计时器 每隔固定时间重新射击
	GetWorldTimerManager().ClearTimer(AutomaticFireTimerHandle);

	//重置后坐力相关变量
	ResetRecoil();

	//更改Fire变量 服务器上改
	ServerStopFire();
	
}

//射线检测方法                                      
void AFPSCharacterBase::RilfeLineTrace(FVector CameraLocation, FRotator CameraRotation, bool IsMoving)
{
	//射线检测结束的位子
	FVector EndLocation;
	//在世界空间中的前向向量
	FVector CameraForWorldVector = UKismetMathLibrary::GetForwardVector(CameraRotation);
	//要忽略的东西
	TArray<AActor*> IgnoreArray;
	IgnoreArray.Add(this);
	//打到了那些
	FHitResult HitResult;
	
	if (ServerPrimryWeapon)
	{
		//是否移动会导致不同EndLocation计算
		if (IsMoving)
		{
			//给xyz全部加一个随机偏移量 
			FVector vector = CameraLocation + CameraForWorldVector * ServerPrimryWeapon->ShootingDistance;
			//简单实现random
			float RandomX = UKismetMathLibrary::RandomFloatInRange(-ServerPrimryWeapon->MovingFireRandomRange,ServerPrimryWeapon->MovingFireRandomRange);
			float RandomY = UKismetMathLibrary::RandomFloatInRange(-ServerPrimryWeapon->MovingFireRandomRange,ServerPrimryWeapon->MovingFireRandomRange);
			float RandomZ = UKismetMathLibrary::RandomFloatInRange(-ServerPrimryWeapon->MovingFireRandomRange,ServerPrimryWeapon->MovingFireRandomRange);
			EndLocation = FVector(vector.X + RandomX, vector.Y + RandomY, vector.Z + RandomZ);

		}
		else
		{
			               //初始位置          前向向量                 距离
			EndLocation = CameraLocation + CameraForWorldVector * ServerPrimryWeapon->ShootingDistance;
		}
		
	}
	
	bool HitSuccess = UKismetSystemLibrary::LineTraceSingle(GetWorld(),CameraLocation,EndLocation,ETraceTypeQuery::TraceTypeQuery1,false,
		IgnoreArray,EDrawDebugTrace::Persistent,HitResult,true,FColor::Red,FColor::Green,
		3.0f);
	if (HitSuccess)
	{
		UKismetSystemLibrary::PrintString(GetWorld(),FString::Printf(TEXT("Hit Actor Name %s"),*HitResult.Actor->GetName()));
		//被击中的物体会强转为玩家累心的指针
		AFPSCharacterBase* FPSCharacter = Cast<AFPSCharacterBase>(HitResult.Actor);
		if (FPSCharacter)
		{
			//打到玩家应用伤害
			//HitResult.Actor是个智能指针所以要Get()方法拿出来	 
			DemagePlayer(HitResult.PhysMaterial->SurfaceType,HitResult.Actor.Get(),CameraLocation,HitResult);
		}
		else
		{
			//和击中目标的法线相关
			FRotator XRotator = UKismetMathLibrary::MakeRotFromX(HitResult.Normal);
			//打到墙生成弹孔 所有玩家都看到 广播弹孔
			MultiSpawnBulletDecal(HitResult.Location,XRotator);
			
		}
		
	}
	
}

void AFPSCharacterBase::FireSniperPrimary()
{
	//UE_LOG(LogTemp,Warning,TEXT("void AFPSCharacterBase::FireWeaponPrimary()"));

	//子弹是否足够 和 不在换弹的时候 才能射击 
	if (ServerPrimryWeapon->CurrentClipAmmo > 0 && !IsReloading && !IsFire)//弹夹里面有多少子弹 大于0才能开枪
	{

		//服务器端执行(枪口闪光粒子效果(能被其他玩家看到听到)(done),播放射击声音(能被其他玩家看到听到)(done),减少弹药(done),创建UI子(done),弹更新(done),射线检测(3种步枪，手枪，狙击枪)，伤害应用，弹孔生成) 
		if (UKismetMathLibrary::VSize(GetVelocity()) > 0.1f)//VSize算出一个准确的长度 根号下里面x2+Y2+Z2
		{
			ServerFireSniperWeapon(PlayerCamera->GetComponentLocation(),PlayerCamera->GetComponentRotation(),true);
		}
		else
		{
			ServerFireSniperWeapon(PlayerCamera->GetComponentLocation(),PlayerCamera->GetComponentRotation(),false);
		}
		
		//客户端执行(墙体播放动画(done)，手臂播放动画(done)，播放射击声音(done)，应用屏幕抖动(done)，应用后座力，枪口闪光粒子效果(done))
		//客户端 (十字线瞄准UI(done) 初始化UI(done)， 播放十字线瞄准扩散动画(done))
		ClientFire();
		
	}
}

void AFPSCharacterBase::StopFireSniperPrimary()
{
	
}


void AFPSCharacterBase::SniperLineTrace(FVector CameraLocation, FRotator CameraRotation, bool IsMoving)
{
	FVector EndLocation;
	//在世界空间中的前向向量
	FVector CameraForWorldVector = UKismetMathLibrary::GetForwardVector(CameraRotation);
	//要忽略的东西
	TArray<AActor*> IgnoreArray;
	IgnoreArray.Add(this);
	//打到了那些
	FHitResult HitResult;
	
	if (ServerPrimryWeapon)
	{
		if (IsAiming)//是否实在开镜瞄准 导致不同的射线检测
		{
			//是否移动会导致不同EndLocation计算
			if (IsMoving)
			{
				//给xyz全部加一个随机偏移量 
				FVector vector = CameraLocation + CameraForWorldVector * ServerPrimryWeapon->ShootingDistance;
				//简单实现random
				float RandomX = UKismetMathLibrary::RandomFloatInRange(-ServerPrimryWeapon->MovingFireRandomRange,ServerPrimryWeapon->MovingFireRandomRange);
				float RandomY = UKismetMathLibrary::RandomFloatInRange(-ServerPrimryWeapon->MovingFireRandomRange,ServerPrimryWeapon->MovingFireRandomRange);
				float RandomZ = UKismetMathLibrary::RandomFloatInRange(-ServerPrimryWeapon->MovingFireRandomRange,ServerPrimryWeapon->MovingFireRandomRange);
				EndLocation = FVector(vector.X + RandomX, vector.Y + RandomY, vector.Z + RandomZ);

			}
			else
			{
				//初始位置          前向向量                 距离
				EndLocation = CameraLocation + CameraForWorldVector * ServerPrimryWeapon->ShootingDistance;
			}
			//射击结束就关闭镜
			ClientEndAiming();
		
		}
		else
		{
			//没开镜瞄准 给一个随即弹道
			//给xyz全部加一个随机偏移量 
			FVector vector = CameraLocation + CameraForWorldVector * ServerPrimryWeapon->ShootingDistance;
			//简单实现random
			float RandomX = UKismetMathLibrary::RandomFloatInRange(-ServerPrimryWeapon->MovingFireRandomRange,ServerPrimryWeapon->MovingFireRandomRange);
			float RandomY = UKismetMathLibrary::RandomFloatInRange(-ServerPrimryWeapon->MovingFireRandomRange,ServerPrimryWeapon->MovingFireRandomRange);
			float RandomZ = UKismetMathLibrary::RandomFloatInRange(-ServerPrimryWeapon->MovingFireRandomRange,ServerPrimryWeapon->MovingFireRandomRange);
			EndLocation = FVector(vector.X + RandomX, vector.Y + RandomY, vector.Z + RandomZ);
			
		}
	
		bool HitSuccess = UKismetSystemLibrary::LineTraceSingle(GetWorld(),CameraLocation,EndLocation,ETraceTypeQuery::TraceTypeQuery1,false,
			IgnoreArray,EDrawDebugTrace::Persistent,HitResult,true,FColor::Red,FColor::Green,
			3.0f);
		if (HitSuccess)
		{
			UKismetSystemLibrary::PrintString(GetWorld(),FString::Printf(TEXT("Hit Actor Name %s"),*HitResult.Actor->GetName()));
			//被击中的物体会强转为玩家累心的指针
			AFPSCharacterBase* FPSCharacter = Cast<AFPSCharacterBase>(HitResult.Actor);
			if (FPSCharacter)
			{
				//打到玩家应用伤害
				//HitResult.Actor是个智能指针所以要Get()方法拿出来	 
				DemagePlayer(HitResult.PhysMaterial->SurfaceType,HitResult.Actor.Get(),CameraLocation,HitResult);
			}
			else
			{
				//和击中目标的法线相关
				FRotator XRotator = UKismetMathLibrary::MakeRotFromX(HitResult.Normal);
				//打到墙生成弹孔 所有玩家都看到 广播弹孔
				MultiSpawnBulletDecal(HitResult.Location,XRotator);
			
			}
		
		}
	
			
	}
		
}

void AFPSCharacterBase::DelaySniperShootCallBack()
{
	IsFire = false;
}


void AFPSCharacterBase::FireWeaponSecondary()
{
	//UE_LOG(LogTemp,Warning,TEXT("void AFPSCharacterBase::FireWeaponPrimary()"));

	//子弹是否足够 和 不在换弹的时候 才能射击 
	if (ServerSecondaryWeapon->CurrentClipAmmo > 0 && !IsReloading)//弹夹里面有多少子弹 大于0才能开枪
	{
		//服务器端执行(枪口闪光粒子效果(能被其他玩家看到听到)(done),播放射击声音(能被其他玩家看到听到)(done),减少弹药(done),创建UI子(done),弹更新(done),射线检测(3种步枪，手枪，狙击枪)，伤害应用，弹孔生成) 
		if (UKismetMathLibrary::VSize(GetVelocity()) > 0.1f)//VSize算出一个准确的长度 根号下里面x2+Y2+Z2
		{
			ServerFirePistolWeapon(PlayerCamera->GetComponentLocation(),PlayerCamera->GetComponentRotation(),true);
		}
		else
		{
			ServerFirePistolWeapon(PlayerCamera->GetComponentLocation(),PlayerCamera->GetComponentRotation(),false);
		}
		
		//客户端执行(墙体播放动画(done)，手臂播放动画(done)，播放射击声音(done)，应用屏幕抖动(done)，应用后座力，枪口闪光粒子效果(done))
		//客户端 (十字线瞄准UI(done) 初始化UI(done)， 播放十字线瞄准扩散动画(done))
		ClientFire();
		//后坐力不需要
		
		//不需要连发
	}
		
}

void AFPSCharacterBase::StopFireSecondary()
{

	ServerStopFire();
}

void AFPSCharacterBase::PistolLineTrace(FVector CameraLocation, FRotator CameraRotaion, bool IsMoving)
{
	FVector EndLocation;
	//在世界空间中的前向向量
	FVector CameraForWorldVector;
	//要忽略的东西
	TArray<AActor*> IgnoreArray;
	IgnoreArray.Add(this);
	//打到了那些
	FHitResult HitResult;
	
	if (ServerSecondaryWeapon)
	{
		//是否移动会导致不同EndLocation计算
		if (IsMoving)
		{
			//手枪后坐力算法 旋转加一个随机便宜 根据连续射击的快慢决定 连续越快 子弹偏移越
			FRotator Rotator;
			Rotator.Roll = CameraRotaion.Roll;//和Z轴没关系
			//摄像机旋转的Pitch和Yaw都会改变 随着PistolSpreadMax PistolSpreadMin越大 偏移越多
			Rotator.Pitch = CameraRotaion.Pitch + UKismetMathLibrary::RandomFloatInRange(PistolSpreadMin,PistolSpreadMax);//和X轴
			Rotator.Yaw = CameraRotaion.Yaw + UKismetMathLibrary::RandomFloatInRange(PistolSpreadMin,PistolSpreadMax);//和Y轴
			//在世界空间中的前向向量
			CameraForWorldVector = UKismetMathLibrary::GetForwardVector(Rotator);
			
			//给xyz全部加一个随机偏移量 
			FVector vector = CameraLocation + CameraForWorldVector * ServerSecondaryWeapon->ShootingDistance;
			//简单实现random
			float RandomX = UKismetMathLibrary::RandomFloatInRange(-ServerSecondaryWeapon->MovingFireRandomRange,ServerSecondaryWeapon->MovingFireRandomRange);
			float RandomY = UKismetMathLibrary::RandomFloatInRange(-ServerSecondaryWeapon->MovingFireRandomRange,ServerSecondaryWeapon->MovingFireRandomRange);
			float RandomZ = UKismetMathLibrary::RandomFloatInRange(-ServerSecondaryWeapon->MovingFireRandomRange,ServerSecondaryWeapon->MovingFireRandomRange);
			EndLocation = FVector(vector.X + RandomX, vector.Y + RandomY, vector.Z + RandomZ);
	
		}
		else
		{
			
			
			//手枪后坐力算法 旋转加一个随机便宜 根据连续射击的快慢决定 连续越快 子弹偏移越
			FRotator Rotator;
			Rotator.Roll = CameraRotaion.Roll;//和Z轴没关系
			//摄像机旋转的Pitch和Yaw都会改变 随着PistolSpreadMax PistolSpreadMin越大 偏移越多
			Rotator.Pitch = CameraRotaion.Pitch + UKismetMathLibrary::RandomFloatInRange(PistolSpreadMin,PistolSpreadMax);//和X轴
			Rotator.Yaw = CameraRotaion.Yaw + UKismetMathLibrary::RandomFloatInRange(PistolSpreadMin,PistolSpreadMax);//和Y轴
			//在世界空间中的前向向量
			CameraForWorldVector = UKismetMathLibrary::GetForwardVector(Rotator);
			                //初始位置          前向向量                 距离
			EndLocation = CameraLocation + CameraForWorldVector * ServerSecondaryWeapon->ShootingDistance;
		}
		
	}
	
	bool HitSuccess = UKismetSystemLibrary::LineTraceSingle(GetWorld(),CameraLocation,EndLocation,ETraceTypeQuery::TraceTypeQuery1,false,
		IgnoreArray,EDrawDebugTrace::Persistent,HitResult,true,FColor::Red,FColor::Green,
		3.0f);

	//设计结束后  偏移量变换
	PistolSpreadMax += ServerSecondaryWeapon->SpreadWeaponMaxIndex;
	PistolSpreadMin -= ServerSecondaryWeapon->SpreadWeaponMinIndex;

	if (HitSuccess)
	{
		UKismetSystemLibrary::PrintString(GetWorld(),FString::Printf(TEXT("Hit Actor Name %s"),*HitResult.Actor->GetName()));
		//被击中的物体会强转为玩家累心的指针
		AFPSCharacterBase* FPSCharacter = Cast<AFPSCharacterBase>(HitResult.Actor);
		if (FPSCharacter)
		{
			//打到玩家应用伤害
			//HitResult.Actor是个智能指针所以要Get()方法拿出来	 
			DemagePlayer(HitResult.PhysMaterial->SurfaceType,HitResult.Actor.Get(),CameraLocation,HitResult);
		}
		else
		{
			//和击中目标的法线相关
			FRotator XRotator = UKismetMathLibrary::MakeRotFromX(HitResult.Normal);
			//打到墙生成弹孔 所有玩家都看到 广播弹孔
			MultiSpawnBulletDecal(HitResult.Location,XRotator);
			
		}
		
	}
	
}

//计时器延迟
void AFPSCharacterBase::DelaySpreadWeaponShootCallBack()
{
	PistolSpreadMin = 0.0f;
	PistolSpreadMax = 0.0f;
}




void AFPSCharacterBase::DelayPlayArmReloadCallBack()
{
	AWeaponBaseServer* CurrentServerWeapon = GetCurrentServerTPBodysWeaponActor();
	if (CurrentServerWeapon)
	{
		//UKismetSystemLibrary::PrintString(this,FString::Printf(TEXT("DelayPlayArmReloadCallBack()")));
		//编程栈上的局部变量 
		int32 GunCurrentAmmo  = CurrentServerWeapon->CurrentGunAmmo;
		int32 ClipCurrentAmmo = CurrentServerWeapon->CurrentClipAmmo;
		int32 const MaxClipAmmo = CurrentServerWeapon->MaxClipAmmo;//不要改
		IsReloading = false;
		//是否装填全部枪械子弹
		//MaxClipAmmo - ClipCurrentAmmo 我需要被装填的子弹数量
		if (MaxClipAmmo - ClipCurrentAmmo >= GunCurrentAmmo)//全部装填
			{   //全部装填
			ClipCurrentAmmo += GunCurrentAmmo;
			GunCurrentAmmo = 0;
			}
		else
		{
			GunCurrentAmmo -= MaxClipAmmo - ClipCurrentAmmo;
			ClipCurrentAmmo = MaxClipAmmo;
		}
		//丢到地上捡起来属性还在
		CurrentServerWeapon->CurrentGunAmmo = GunCurrentAmmo;
		CurrentServerWeapon->CurrentClipAmmo = ClipCurrentAmmo;
		//更新UI
		ClientUpdateAmmoUI(ClipCurrentAmmo,GunCurrentAmmo);
	}


}

void AFPSCharacterBase::DemagePlayer(EPhysicalSurface Surface,AActor* DamagedActor,FVector& HitFromDirection,FHitResult& HitInfo)
{	//五个位置会应用不同的伤害
	float BaseDamage = 0;	
	if (ServerPrimryWeapon)
	{
		BaseDamage = ServerPrimryWeapon->BaseDamage;
	}
	else if (ServerSecondaryWeapon)
	{
		BaseDamage = ServerSecondaryWeapon->BaseDamage;
	}
	
	//更具表面材质 做不同的伤害
	switch (Surface)
	{
	case EPhysicalSurface::SurfaceType1:
		{
			//Head
			UGameplayStatics::ApplyPointDamage(DamagedActor,BaseDamage * 4,HitFromDirection,HitInfo,
				GetController(),
				this,
				UDamageType::StaticClass());//打了别人调用这个 发通知
		}
		break;
	case EPhysicalSurface::SurfaceType2:
		{
			//Body
			UGameplayStatics::ApplyPointDamage(DamagedActor,BaseDamage * 1,HitFromDirection,HitInfo,
				GetController(),
				this,
				UDamageType::StaticClass());//打了别人调用这个 发通知
		}
		break;
	case EPhysicalSurface::SurfaceType3:
		{
			//Arm
			UGameplayStatics::ApplyPointDamage(DamagedActor,BaseDamage * 0.8,HitFromDirection,HitInfo,
				GetController(),
				this,
				UDamageType::StaticClass());//打了别人调用这个 发通知
		}
		break;
	case EPhysicalSurface::SurfaceType4:
		{
			//Leg
			UGameplayStatics::ApplyPointDamage(DamagedActor,BaseDamage * 0.7,HitFromDirection,HitInfo,
				GetController(),
				this,
				UDamageType::StaticClass());//打了别人调用这个 发通知
		}
		break;
		
	}
	
	


}

//被受到伤害的回调	  服务器上调用了
void AFPSCharacterBase::OnHit(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation,
	UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const UDamageType* DamageType,
	AActor* DamageCauser)
{
	Health -= Damage;
	//UKismetSystemLibrary::PrintString(this,FString::Printf(TEXT("PlayerName : %s　-- Health : %f"),*GetName(),Health));
	//1.客户端RPC 2.调用客户端PlayerController的一个方法(留给蓝图实现) 3.实现PlayerUI里面的血量减少接口
	ClientUpdateHealthUI(Health);
	if (Health <= 0)
	{
		//死亡逻辑
		DeathShootDeath(DamageCauser);
		
	}
	
}

#pragma endregion 
