// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "CombatComponent.generated.h"

class AWeapon;
class ABlasterCharacter;
class ABlasterPlayerController;
class ABlasterHUD;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//使用友元类会使得BlasterCharacter可以有完全的权限访问CombatComponent，私有成员和保护成员都能访问。
	friend class ABlasterCharacter;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//蓝图里面调用，当换弹动画完成时使用
	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	UFUNCTION()
	void Fire(bool bFire);

	UFUNCTION(BlueprintCallable)
	void ShotGunReload();
	void JumpToShotGunEnd();

	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();
	UFUNCTION(BlueprintCallable)
	void LauncherGrenade();
	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target);

	UFUNCTION(BlueprintCallable)
	void FinishSwap();
	UFUNCTION(BlueprintCallable)
	void FinishSwapAttachWeapons();
	bool bIsLocallySwap = false;
	
	/*
	 * 拾起物品
	 */
	void PickupAmmo(EWeaponType WeaponTpye, int32 AmmoAmmount);

	
protected:
	virtual void BeginPlay() override;
	//我们将bAiming作为复制变量，但是这只针对服务端到客户端，对于客户端到服务端，我们需要使用服务端RPC
	
	void SetAiming(bool bIsAiming);
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	void EquipWeapon(AWeapon* WeaponToEquip);
	UFUNCTION()
	void OnRep_EquippedWeapon();
	UFUNCTION()
	void OnRep_EquippedSecondaryWeapon();
	
	UFUNCTION()
	void Reload();
	bool bIsLocalReload = false;
	UFUNCTION(Server, Reliable)
	void ServerReload();
	void HandleReload();
	int32 AmountToReload();
	
	UFUNCTION()
	void Fire_ButtonPreesed();

	void FireProjectileWeapon();
	void FireHitScanWeapon();
	void FireShotGun();
	
	void LocalFire(const FVector_NetQuantize& TraceHitTarget);
	void ShotGunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTarget);
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerShotGunFire(const TArray<FVector_NetQuantize>& TraceHitTarget, float FireDelay, bool bAutomatic);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastShotGunFire(const TArray<FVector_NetQuantize>& TraceHitTarget);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget, float FireDelay, bool bAutomatic);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void ThrowGrenade();
	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> GrenadeClass;
	
	void TraceUnderCrosshairs(FHitResult& TraceHitResult);
	void SetHUDCrosshairs(float DeltaTime);

	/*
	 * 代码重构
	 */
	void DropCurrentWeapon();
	void AttachToRightHand(AActor* ActorToAttach);
	void AttachToLeftHand(AActor* ActorToAttach);
	void AttachToBagBack(AActor* ActorToAttach);
	void UpdateCarriedAmmo();
	void PlayEquipSound(AWeapon* WeaponToEquip);
	void ReloadEmptyWeapon();
	void ShowAttachedGrenade(bool bShowGrenade);

	/*
	 *	装备主武器和副武器
	 */
	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);
	void SwapWeapon();
	
	
private:
	UPROPERTY()
	ABlasterCharacter* Character;
	UPROPERTY()
	ABlasterPlayerController* Controller;
	UPROPERTY()
	ABlasterHUD* HUD;
	
	UPROPERTY(ReplicatedUsing= OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;
	UPROPERTY(ReplicatedUsing= OnRep_EquippedSecondaryWeapon)
	AWeapon* SecondaryWeapon;
	
	UPROPERTY(ReplicatedUsing= OnRep_Aiming)
	bool bAiming = false;
	bool bAimButtonPressed = false;
	UFUNCTION()
	void OnRep_Aiming();
	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	UPROPERTY(EditAnywhere)
	bool bIsFire;

	/*
	 * HUD 与准星
	 */
	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairFireFactor;

	UPROPERTY()
	FVector HitTarget;

	UPROPERTY()
	FHUDPackage HUDPackage;
	/**
	 * Aiming and FOV
	 */
	// 不瞄准时我们的视野，将会在游戏开始时设置到相机的初始FOV中
	float DefaultFOV;
	UPROPERTY(EditAnywhere, Category="Combat")
	float ZoomedFOV = 30.f;
	float CurrentFOV;
	UPROPERTY(EditAnywhere, Category="Combat")
	float ZoomInterpSpeed = 20.f;
	void InterpFOV(float DeltaTime);
	
	/*
	 *  自动开火
	 */
	FTimerHandle FireTimer;
	bool bCanFire = true;
	void StartFireTimer();
	void FireTimerFinished();
	bool CanFire();

	//角色在当前武器中携带的弹药
	UPROPERTY(ReplicatedUsing= OnRep_CarriedAmmo)
	int32 CarriedAmmo;
	UFUNCTION()
	void OnRep_CarriedAmmo();
	TMap<EWeaponType, int32> CarriedAmmoMap;

	/*
	 * 各类武器初始弹药
	 */
	UPROPERTY(EditDefaultsOnly)
	int32 StartingARAmmo = 30;
	UPROPERTY(EditDefaultsOnly)
	int32 StartingRocketAmmo = 0;
	UPROPERTY(EditDefaultsOnly)
	int32 StartingPistolAmmo = 15;
	UPROPERTY(EditDefaultsOnly)
	int32 StartingSMGAmmo = 40;
	UPROPERTY(EditDefaultsOnly)
	int32 StartingShotGunAmmo = 16;
	UPROPERTY(EditDefaultsOnly)
	int32 StartingSniperRifleAmmo = 10;
	UPROPERTY(EditDefaultsOnly)
	int32 StartingGrenadesLauncherAmmo = 8;
	void InitializeCarriedAmmo();
	
	UPROPERTY(ReplicatedUsing= OnRep_Grenades)
	int32 Grenades = 4;
	UPROPERTY(EditAnywhere)
	int32 MaxGrenades = 4;

	UFUNCTION()
	void OnRep_Grenades();

	void UpdateHuDGrenade();
	
	UPROPERTY(ReplicatedUsing= OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;
	UFUNCTION()
	void OnRep_CombatState();

	void UpdateAmmoValue();
	void UpdateShotGunAmmoValue();
	
public:	
	FORCEINLINE int32 GetNumGrenades() const { return Grenades; }
};


