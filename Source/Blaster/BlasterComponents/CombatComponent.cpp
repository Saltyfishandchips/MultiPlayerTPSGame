// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Camera/CameraComponent.h"
#include "Sound/SoundCue.h"
#include "Blaster/Character/BlasterAnimInstance.h"
#include "Blaster/Weapon/Projectile.h"
#include "Blaster/Weapon/ShotGunWeapon.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 450.f;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

		if (Character->GetFollowCamera())
		{
			DefaultFOV = Character->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}

		//初始化携带弹药
		if (Character->HasAuthority())
		{
			InitializeCarriedAmmo();
		}
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		
		//Tick里面一直启用射线检测
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
	}

}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		//获取游戏视口二维坐标
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);
	
	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;

		if (Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
		}
		
		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;
		bool bHasTarget = GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);

		if (!bHasTarget)
		{
			TraceHitResult.ImpactPoint = End;
		}

		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{
			HUDPackage.CrosshairColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairColor = FLinearColor::White;
		}
	}
	
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;

	//Cast只会在初始化时使用一次，对于性能不会产生太大影响
	Controller = Controller == nullptr? Cast<ABlasterPlayerController>(Character->Controller): Controller;

	if (Controller)
	{
		HUD = HUD == nullptr? Cast<ABlasterHUD>(Controller->GetHUD()): HUD;
		if (HUD)
		{
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
			}

			//计算准星偏移

			//将走路速度映射[0, 600] -> [0, 1]
			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;
			
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

			if (Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}

			if (bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, DeltaTime, 30.f);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}
	
			CrosshairFireFactor = FMath::FInterpTo(CrosshairFireFactor, 0.f, DeltaTime, 35.f); 
			
			HUDPackage.CrossharirSpread = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairFireFactor;
			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return;

	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomedInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	if (Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	bAiming = bIsAiming;
	//服务端RPC无论是服务端运行还是客户端运行都会在服务端执行RPC，因此我们不需要检查是否为服务端
	ServerSetAiming(bIsAiming);

	if(Character)	
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming? AimWalkSpeed: BaseWalkSpeed;
	}

	// 狙击枪开启瞄准镜
	if (Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		Character->ShowSniperScopeWidget(bIsAiming);
	}

	if (Character->IsLocallyControlled())
	{
		bAimButtonPressed = bIsAiming;
	}
}

void UCombatComponent::OnRep_Aiming()
{
	if (Character && Character->IsLocallyControlled())
	{
		bAiming = bAimButtonPressed;
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;

	if(Character)	
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming? AimWalkSpeed: BaseWalkSpeed;
	}		
}

void UCombatComponent::OnRep_CombatState()
{
	/*
	 * 当Combat的状态改变时，客户端会自动调用
	 * 即我们可以在服务端改变状态，客户端将会自动改变
	 */
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading:
		if(Character && !Character->IsLocallyControlled()) HandleReload();
		break;
	case ECombatState::ECS_Unoccupied:
		if (bIsFire)
		{
			Fire_ButtonPreesed();
		}
		break;
	case ECombatState::ECS_ThrowGrenade:
		if (Character && !Character->IsLocallyControlled())
		{
			AttachToLeftHand(EquippedWeapon);
			ShowAttachedGrenade(true);
			Character->PlayThrowGrenadeMontage();
		}
		break;
	case ECombatState::ECS_SwapWeapon:
		if (Character && !Character->IsLocallyControlled())
		{
			Character->PlaySwapMontage();
		}
		break;
	case ECombatState::ECS_MAX:
		break;
	}
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;
	if (CombatState != ECombatState::ECS_Unoccupied) return;

	if (EquippedWeapon != nullptr && SecondaryWeapon == nullptr)
	{
		EquipSecondaryWeapon(WeaponToEquip);
	}
	else
	{
		EquipPrimaryWeapon(WeaponToEquip);
	}
	
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		
		AttachToRightHand(EquippedWeapon);
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;

		//客户端本地播放声音
		PlayEquipSound(EquippedWeapon);
		EquippedWeapon->SetHUDAmmo();
	}
}

void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	DropCurrentWeapon();
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	AttachToRightHand(EquippedWeapon);
	//当我们装备上了武器以后，武器也需要有Owner，否则武器的所有者是没有的,这个是会被自动复制的(前提是actor是replicate的)
	EquippedWeapon->SetOwner(Character);
	//服务端更新HUD中的Ammo数量，客户端直接用OnRep_Owner
	EquippedWeapon->SetHUDAmmo();

	UpdateCarriedAmmo();

	//服务器上调用Server，只会在服务器上运行，因此服务端能听到声音
	//客户端上调用Server，也只会在服务器上运行
	PlayEquipSound(WeaponToEquip);
	ReloadEmptyWeapon();
}

void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	SecondaryWeapon = WeaponToEquip;
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachToBagBack(WeaponToEquip);
	PlayEquipSound(WeaponToEquip);

	if (EquippedWeapon == nullptr) return;
	SecondaryWeapon->SetOwner(Character);
}

//在ServerRPC中调用的，但是在客户端用OnRep进行同步
void UCombatComponent::SwapWeapon()
{
	if (CombatState != ECombatState::ECS_Unoccupied || Character == nullptr) return;
	if (bIsLocalReload) return;
	
	if (Character->IsLocallyControlled())
	{
		bIsLocallySwap = true;
	}
	
	Character->PlaySwapMontage();
	CombatState = ECombatState::ECS_SwapWeapon;
	
	AWeapon* Temp = EquippedWeapon;
	EquippedWeapon = SecondaryWeapon;
	SecondaryWeapon = Temp;
	
}

void UCombatComponent::OnRep_EquippedSecondaryWeapon()
{
	if (SecondaryWeapon && Character)
	{
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
		AttachToBagBack(SecondaryWeapon);
		PlayEquipSound(EquippedWeapon);
	}
}

void UCombatComponent::DropCurrentWeapon()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Drop();
	}
}

void UCombatComponent::AttachToRightHand(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr ) return;
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttachToLeftHand(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr || EquippedWeapon == nullptr) return;
	bool bUsePistolSocket = EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol || EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SMG;
	FName SocketName = bUsePistolSocket? FName("PistolSocket"): FName("LeftHandSocket");
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(SocketName);
	if (Character && Character->GetMesh() && HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttachToBagBack(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr ) return;
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("EquipSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::UpdateCarriedAmmo()
{
	if (EquippedWeapon == nullptr) return;
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	
	Controller = Controller == nullptr? Cast<ABlasterPlayerController>(Character->Controller): Controller;
	//服务端更新HUD中的CarriedAmmo数量
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::PlayEquipSound(AWeapon* WeaponToEquip)
{
	if (Character && WeaponToEquip && WeaponToEquip->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			WeaponToEquip->EquipSound,
			Character->GetActorLocation());
	}
}

void UCombatComponent::ReloadEmptyWeapon()
{
	if (EquippedWeapon && EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

void UCombatComponent::ShowAttachedGrenade(bool bShowGrenade)
{
	if (Character && Character->GetGrenadeMesh())
	{
		Character->GetGrenadeMesh()->SetVisibility(bShowGrenade);
	}
}

void UCombatComponent::Reload()
{
	if (CarriedAmmo > 0 && CombatState == ECombatState::ECS_Unoccupied && EquippedWeapon && !EquippedWeapon->IsFull() && !bIsLocalReload)
	{
		ServerReload();
		HandleReload();
		bIsLocalReload = true;
	}
}

void UCombatComponent::ServerReload_Implementation()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	//服务端执行Reload的蒙太奇，客户端则是通过OnRep执行
	CombatState = ECombatState::ECS_Reloading;
	if (!Character->IsLocallyControlled()) HandleReload();
}

void UCombatComponent::HandleReload()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	Character->PlayReloadMontage();
}

/*
 * 专门用于处理霰弹枪的换弹
 */
void UCombatComponent::ShotGunReload()
{
	if (Character && Character->HasAuthority())
	{
		UpdateShotGunAmmoValue();	
	}
}

void UCombatComponent::UpdateShotGunAmmoValue()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	
	//计算需要装填的子弹数量
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller = Controller == nullptr? Cast<ABlasterPlayerController>(Character->Controller): Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	
	EquippedWeapon->AddAmmo(1);
	//霰弹枪一旦装弹即可发射
	bCanFire = true;
	if (EquippedWeapon->IsFull() || CarriedAmmo == 0)
	{
		//跳至换弹蒙太奇的最后
		UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(Character->GetReloadMontage());
			AnimInstance->Montage_JumpToSection(FName("ShotGunEnd"));
		}
	}
}

//客户端执行
void UCombatComponent::JumpToShotGunEnd()
{
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(Character->GetReloadMontage());
		AnimInstance->Montage_JumpToSection(FName("ShotGunEnd"));
	}
}

void UCombatComponent::UpdateAmmoValue()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	int32 ReloadAmount = AmountToReload();
	//计算需要装填的子弹数量
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	//客户端由于Ammo的变化调用OnRep_Ammo(Weapon.cpp)
	//服务端则是通过下面的函数显示更新后的子弹数量
	Controller = Controller == nullptr? Cast<ABlasterPlayerController>(Character->Controller): Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	
	EquippedWeapon->AddAmmo(ReloadAmount);
}



int32 UCombatComponent::AmountToReload()
{
	if (EquippedWeapon == nullptr) return 0;
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();
	
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(AmountCarried, RoomInMag);
		return FMath::Clamp(RoomInMag, 0, Least);
	}
	return 0;
}

//在播放完换弹动画后，在动画蓝图中调用
void UCombatComponent::FinishReloading()
{
	if (Character == nullptr) return;
	bIsLocalReload = false;
	if (Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValue();
	}
	if (bIsFire)
	{
		Fire_ButtonPreesed();
	}
}

void UCombatComponent::PickupAmmo(EWeaponType WeaponTpye, int32 AmmoAmmount)
{
	if (CarriedAmmoMap.Contains(WeaponTpye))
	{
		CarriedAmmoMap[WeaponTpye] += AmmoAmmount;
		UpdateCarriedAmmo();
	}
	if (EquippedWeapon && EquippedWeapon->IsEmpty() && EquippedWeapon->GetWeaponType() == WeaponTpye)
	{
		Reload();
	}
}

bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr) return false;
	if (!EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_ShotGun) return true;
	if (bIsLocalReload) return false;
	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;
}

void UCombatComponent::Fire(bool bFire)
{
	bIsFire = bFire;

	if (bIsFire)
	{
		Fire_ButtonPreesed();
	}

}

void UCombatComponent::Fire_ButtonPreesed()
{
	if (CanFire())
	{
		bCanFire = false;
		if (EquippedWeapon)
		{
			CrosshairFireFactor = 1.f;
			//根据武器是否为Projectile还是HitScan来调用不同的函数
			//HitScan的特殊点在于客户端上Scatter的结果需要Replicate到服务端上
			switch (EquippedWeapon->FireType)
			{
			case EFireType::EFT_Projectile:
				FireProjectileWeapon();
				break;
			case EFireType::EFT_HitScan:
				FireHitScanWeapon();
				break;
			case EFireType::EFT_Shotgun:
				FireShotGun();
				break;
			case EFireType::EFT_MAX:
				break;
			}
		}
		StartFireTimer();
	}
}


void UCombatComponent::FireProjectileWeapon()
{
	HitTarget = EquippedWeapon->bUseScatter? EquippedWeapon->SetHitScatter(HitTarget): HitTarget;
	//确保只有客户端会调用LocalFire，否则服务端会调用LocalFire和ServerFire两次
	if (Character && !Character->HasAuthority())
	{
		LocalFire(FVector_NetQuantize(HitTarget));
	}
	ServerFire(FVector_NetQuantize(HitTarget), EquippedWeapon->FireDelay, EquippedWeapon->bAutomatic);
}

void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) return;

	//霰弹枪的在换弹后可立即击发
	// if (Character && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_ShotGun)
	// {
	// 	Character->PlayFireMontage(bIsFire);
	// 	EquippedWeapon->Fire(TraceHitTarget);
	// 	CombatState = ECombatState::ECS_Unoccupied;
	// 	return;
	// }
	
	if (Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bIsFire);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget, float FireDelay, bool bAutomatic)
{
	//Multicast则是会在服务端中广播给所有客户端
	MulticastFire(TraceHitTarget);
}

bool UCombatComponent::ServerFire_Validate(const FVector_NetQuantize& TraceHitTarget, float FireDelay, bool bAutomatic)
{
	if (EquippedWeapon == nullptr) return true;
	const bool NearlyClose = FMath::IsNearlyEqual(EquippedWeapon->FireDelay, FireDelay, 0.001f);
	const bool CheckAutomatic = EquippedWeapon->bAutomatic == bAutomatic;
	return NearlyClose && CheckAutomatic;
}


void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	//本地不需要执行，放在了LocalFire，以提高响应速度
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	LocalFire(TraceHitTarget);
}

void UCombatComponent::FireHitScanWeapon()
{
	HitTarget = EquippedWeapon->bUseScatter? EquippedWeapon->SetHitScatter(HitTarget): HitTarget;
	if (Character && !Character->HasAuthority())
	{
		LocalFire(FVector_NetQuantize(HitTarget));
	}
	ServerFire(FVector_NetQuantize(HitTarget), EquippedWeapon->FireDelay, EquippedWeapon->bAutomatic);
}

void UCombatComponent::FireShotGun()
{
	AShotGunWeapon* ShotGun = Cast<AShotGunWeapon>(EquippedWeapon);
	if (ShotGun && Character)
	{
		TArray<FVector_NetQuantize> HitTargetArray;
		ShotGun->ShotGunTraceEndWithScatter(HitTarget, HitTargetArray);
		if (!Character->HasAuthority()) ShotGunLocalFire(HitTargetArray);
		ServerShotGunFire(HitTargetArray, EquippedWeapon->FireDelay, EquippedWeapon->bAutomatic);
	}
}

void UCombatComponent::ShotGunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTarget)
{
	AShotGunWeapon* ShotGun = Cast<AShotGunWeapon>(EquippedWeapon);
	if(ShotGun == nullptr || Character == nullptr) return;
	//霰弹枪的在换弹后可立即击发
	if (CombatState == ECombatState::ECS_Reloading || CombatState == ECombatState::ECS_Unoccupied)
	{
		bIsLocalReload = false;
		Character->PlayFireMontage(bIsFire);
		ShotGun->FireShotGun(TraceHitTarget);
		CombatState = ECombatState::ECS_Unoccupied;
	}
}

void UCombatComponent::ServerShotGunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTarget, float FireDelay, bool bAutomatic)
{
	MulticastShotGunFire(TraceHitTarget);
}

bool UCombatComponent::ServerShotGunFire_Validate(const TArray<FVector_NetQuantize>& TraceHitTarget, float FireDelay, bool bAutomatic)
{
	if (EquippedWeapon == nullptr) return true;
	const bool NearlyClose = FMath::IsNearlyEqual(EquippedWeapon->FireDelay, FireDelay, 0.001f);
	const bool CheckAutomatic = EquippedWeapon->bAutomatic == bAutomatic;
	return NearlyClose && CheckAutomatic;
}

void UCombatComponent::MulticastShotGunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTarget)
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	ShotGunLocalFire(TraceHitTarget);
}

void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		EquippedWeapon->FireDelay);
}

void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr || Character == nullptr) return;
	bCanFire = true;
	if (bIsFire && EquippedWeapon->bAutomatic)
	{
		Fire_ButtonPreesed();
	}
	ReloadEmptyWeapon();
}

void UCombatComponent::ThrowGrenade()
{
	if (Grenades == 0) return;
	//确保Grenades必须执行完才能再次进入
	if (EquippedWeapon == nullptr || CombatState != ECombatState::ECS_Unoccupied) return;
	
	CombatState = ECombatState::ECS_ThrowGrenade;
	//客户端本地执行，服务端控制的Character执行
	//同时客户端会调用ServerRPC让服务端执行ServerThrowGrenade
	if (Character)
	{
		//本地调用
		Character->PlayThrowGrenadeMontage();
		AttachToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}
	//防止Server再次调用自己，只有Client调用ServerRPC
	if (Character && !Character->HasAuthority())
	{
		ServerThrowGrenade();
	}
	if (Character && Character->HasAuthority())
	{
		Grenades = FMath::Clamp(Grenades - 1,0, MaxGrenades);
		UpdateHuDGrenade();
	}

}

void UCombatComponent::ServerThrowGrenade_Implementation()
{
	if (CombatState == ECombatState::ECS_ThrowGrenade) return;
	//服务端的检查
	if (Grenades == 0) return;
	CombatState = ECombatState::ECS_ThrowGrenade;
	if (Character)
	{
		AttachToLeftHand(EquippedWeapon);
		Character->PlayThrowGrenadeMontage();
		ShowAttachedGrenade(true);
	}
	
	Grenades = FMath::Clamp(Grenades - 1,0, MaxGrenades);
	UpdateHuDGrenade();
}

void UCombatComponent::ThrowGrenadeFinished()
{
	AttachToRightHand(EquippedWeapon);
	//在所有机器播放了动画后调用
	CombatState = ECombatState::ECS_Unoccupied;
}

void UCombatComponent::LauncherGrenade()
{
	ShowAttachedGrenade(false);
	if (Character && Character->IsLocallyControlled())
	{
		ServerLaunchGrenade(HitTarget);
	}
}

void UCombatComponent::FinishSwap()
{
	if (Character && Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
	}

	if (Character)
	{
		Character->bFinishedSwapping = true;
		bIsLocallySwap = false;
	}
}

void UCombatComponent::FinishSwapAttachWeapons()
{
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachToRightHand(EquippedWeapon);
	
	EquippedWeapon->SetHUDAmmo();
	UpdateCarriedAmmo();
	
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachToBagBack(SecondaryWeapon);

}

void UCombatComponent::ServerLaunchGrenade_Implementation(const FVector_NetQuantize& Target)
{
	//&& Character->HasAuthority() 
	if (Character && GrenadeClass && Character->GetGrenadeMesh())
	{
		const FVector StartingLocation = Character->GetGrenadeMesh()->GetComponentLocation();
		FVector ToTarget = Target - StartingLocation;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		
		UWorld* World = GetWorld();
		if (World)
		{
			World->SpawnActor<AProjectile>(
				GrenadeClass,
				StartingLocation,
				ToTarget.Rotation(),
				SpawnParams
				);
		}
	}
}

void UCombatComponent::OnRep_Grenades()
{
	UpdateHuDGrenade();
}

void UCombatComponent::UpdateHuDGrenade()
{
	Controller = Controller == nullptr? Cast<ABlasterPlayerController>(Character->Controller): Controller;
	if (Controller)
	{
		Controller->SetHUDGrenade(Grenades);
	}
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	Controller = Controller == nullptr? Cast<ABlasterPlayerController>(Character->Controller): Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}

	bool bJumpToShotgunEnd =
		CombatState == ECombatState::ECS_Reloading &&
		EquippedWeapon != nullptr &&
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_ShotGun &&
		CarriedAmmo == 0;
				
	
	if (bJumpToShotgunEnd)
	{
		JumpToShotGunEnd();
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SMG, StartingSMGAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_ShotGun, StartingShotGunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, StartingSniperRifleAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadesLauncher, StartingGrenadesLauncherAmmo);
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME(UCombatComponent, Grenades);
}


