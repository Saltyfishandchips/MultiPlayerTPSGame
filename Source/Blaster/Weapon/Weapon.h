// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "WeaponTypes.h"
#include "Weapon.generated.h"


UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_EquippedSecondary UMETA(DisplayName = "Equipped Secondary"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_HitScan UMETA(DisplayName = "Hit Scan Weapon"),
	EFT_Projectile UMETA(DisplayName = "Projectile Weapon"),
	EFT_Shotgun UMETA(DisplayName = "Shotgun Weapon"),
	EFT_MAX UMETA(DisplayName = "DefaultMAX")
};

class USkeletalMeshComponent;
class USphereComponent;
class UWidgetComponent;
class UAnimationAsset;
class UTexture2D;

UCLASS()
class BLASTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void ShowPickupWidget(const bool bShowWidget) const;

	virtual void Fire(const FVector& HitTarget);
	void Drop();
	void AddAmmo(const int32 AmmoToAdd);
	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);
	/*
	 * 武器中央准星的纹理
	 */
	UPROPERTY(EditAnywhere)
	UTexture2D* CrosshairsCenter;

	UPROPERTY(EditAnywhere)
	UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere)
	UTexture2D* CrosshairsBottom;

	UPROPERTY(EditAnywhere)
	UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere)
	UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* OutLineMesh;
	/*
	 * 自动开火
	 */
	UPROPERTY(EditAnywhere, Category= "Fire")
	float FireDelay = 0.15f;

	UPROPERTY(EditAnywhere, Category= "Fire")
	bool bAutomatic = true;

	void SetHUDAmmo();

	UPROPERTY(EditAnywhere)
	class USoundCue* EquipSound;

	UPROPERTY(EditAnywhere)
	EFireType FireType;

	/*
	 * 用于HitScanWeapon的Scatter同步
	 * 在之前是将其放在HitScanWeapon中进行处理，服务器需要计算出散射点
	 * 现在由本地先计算出散射点，将结果发送至服务端
	 */
	FVector SetHitScatter(const FVector& HitTarget) const;
	UPROPERTY(EditAnywhere, Category="Weapon Scatter")
	bool bUseScatter = false;

protected:
	virtual void BeginPlay() override;
	virtual void OnWeaponStateSet();
	virtual void OnEquipped();
	virtual void OnDropped();
	virtual void OnEquippedSecondary();
	
	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
		);

	UFUNCTION()
	virtual void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
		);

	//绘制球体，并在其中随机取点做射线检测
	UPROPERTY(EditAnywhere, Category="Weapon Scatter")
	float DistanceToSphere = 200.f;
	UPROPERTY(EditAnywhere, Category="Weapon Scatter")
	float SphereRadius = 80.f;

	UPROPERTY(EditAnywhere)
	float HeadShotDamage = 40.f;
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	UPROPERTY(Replicated, EditAnywhere)
	bool bUseServerSideRewind = false;

	UPROPERTY()
	ABlasterCharacter* OwningCharacter;
	UPROPERTY()
	ABlasterPlayerController* OwningPlayerController;

	UFUNCTION()
	void OnPingTooHigh(bool bPingTooHigh);
private:
	UPROPERTY(VisibleAnywhere, Category="Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category="Weapon Properties")
	USphereComponent* SphereComponent;

	UPROPERTY(ReplicatedUsing= OnRep_WeaponState, VisibleAnywhere)
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();
	
	UPROPERTY(VisibleAnywhere, Category="Weapon Properties")
	UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere, Category= "WeaponProperty")
	UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ACasing> CasingClass;

	/*
	 * 瞄准时FOV的大小
	 */

	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;

	// 武器子弹
	// UPROPERTY(EditAnywhere, ReplicatedUsing= OnRep_Ammo)
	UPROPERTY(EditAnywhere)
	int32 Ammo;

	/*
	 *  这里使用Client-Predication
	 */
	void SpendRound();
	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);
	//客户端向服务端发送RPC的次数
	int32 Sequence = 0;
	// UFUNCTION()
	// void OnRep_Ammo();

	virtual void OnRep_Owner() override;
	
	UPROPERTY(EditAnywhere)
	int32 MagCapacity;
	
	//武器类型
	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;
	
public:	
	void SetWeaponState(EWeaponState State);
	FORCEINLINE USphereComponent* GetSphereComponent() const { return  SphereComponent; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomedInterpSpeed() const { return ZoomInterpSpeed; }
	bool IsEmpty();
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const {return Ammo;}
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity;}
	FORCEINLINE bool IsFull() const { return Ammo == MagCapacity; }
	// 只有初始武器设置为true，以便在Character->Elim中调用Destroy，否则地图中的武器将会累积
	bool bShouldDestroyed = false;
	FORCEINLINE float GetDamage() const { return Damage; }
	FORCEINLINE float GetHeadShotDamage() const { return HeadShotDamage; }
};

