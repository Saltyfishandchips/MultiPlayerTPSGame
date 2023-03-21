// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "GameFramework/Character.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "Blaster/BlasterTypes/Team.h"
#include "BlasterCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);

class UCameraComponent;
class USpringArmComponent;
class UWidgetComponent;
class AWeapon;
class UCombatComponent;
class UAnimMontage;
class ABlasterPlayerController;
class AController;
class ABlasterPlayerState;
class UBoxComponent;

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(Server, Reliable)
	void ServerLeaveGame();
	FOnLeftGame OnLeftGame;
	
	ABlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;

	//播放蒙太奇
	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	void PlayElimMotage();
	void PlayHitReactMontage();
	void PlayThrowGrenadeMontage();
	void PlaySwapMontage();
	
	void Elim(bool bPlayerLeftGame);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim(bool bPlayerLeftGame);
	
	virtual void OnRep_ReplicatedMovement() override;

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);

	void UpdateHUDHealth();
	void UpdateHUDShield();

	void SpawnDefaultWeapon();

	UPROPERTY()
	TMap<FName, UBoxComponent*> HitCollisionBoxMap;

	bool bFinishedSwapping = false;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastGainedTheLead();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastLostTheLead();

	void SetTeamColor(ETeam Team);
	
protected:
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	virtual void Jump() override;
	void Turn(float Value);
	void LookUp(float Value);
	
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void CrouchButtonRelease();
	void ReloadButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void ThrowGrenadePressed();
	void FireButtonPressed();
	void FireButtonReleased();
	void SwapWeaponPressed();
	
	void AimOffset(float DeltaTime);
	void SimProxiesTurn();
	void CalculateAO_Pitch();
	void RotateInPlace(float DeltaTime);

	UFUNCTION()
	void ReceiveDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser);

	void PollInit();
	virtual void Destroyed() override;
	void DropOrDestroyWeapon(AWeapon* Weapon);

	UFUNCTION(Client, Reliable)
	void ResetWeaponState();

	void SetSpawnPoint();
	void OnPlayerStateInitialized();
	
	/*
	 * Hit Box 用于Server-Side Rewind
	 */
	UPROPERTY(EditAnywhere)
	UBoxComponent* head;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* pelvis;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_02;

	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_03;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_l;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* backpack;

	UPROPERTY(EditAnywhere)
	UBoxComponent* blanket;

	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_r;


private:
	//Blaster Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess))
	UCombatComponent* CombatComponent;
	UPROPERTY(VisibleAnywhere)
	class UBuffComponent* BuffComponent;
	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess))
	class ULagCompensationComponent* LagCompensationComponent;
	
	UPROPERTY(VisibleAnywhere, Category= "Camera")
	USpringArmComponent* SpringArmComponent;
	
	UPROPERTY(VisibleAnywhere, Category= "Camera")
	UCameraComponent* CameraComponent;

	//AllowPrivateAccess代表我们会将这个私有变量暴露给蓝图使用
	//只有加上这个meta后我们才可以对C++类的私有变量使用BlueprintReadOnly和BlueprintReadWrite
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	UWidgetComponent* OverHeadWidget;

	//与角色重合的武器
	UPROPERTY(ReplicatedUsing= OnRep_OverlappingWeapon)
	AWeapon* OverlappingWeapon;

	//当重合武器改变时，在客户端执行下面的函数
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);
	
	//对于客户端装备武器，我们调用服务端RPC，客户端通知服务端执行该函数
	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	UFUNCTION(Server, Reliable)
	void ServerSwapWeaponPressed();
	
	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	
	FRotator StartAimRotation;
	
	ETurnInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	/*
	 * 动画蒙太奇
	 */
	UPROPERTY(EditAnywhere, Category="Combat")
	UAnimMontage* FireWeaponMontage;
	UPROPERTY(EditAnywhere, Category="Combat")
	UAnimMontage* HitReactMotage;
	UPROPERTY(EditAnywhere, Category="Combat")
	UAnimMontage* ElimMotage;
	UPROPERTY(EditAnywhere, Category="Combat")
	UAnimMontage* ReloadMontage;
	UPROPERTY(EditAnywhere, Category="Combat")
	UAnimMontage* ThrowGrenadeMontage;
	UPROPERTY(EditAnywhere, Category="Combat")
	UAnimMontage* SwapMontage;
	
	void HideCameraIfCharacterClose();
	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;
	float CalcuateSpeed();

	/*
	 * 玩家生命值 
	 */
	UPROPERTY(EditAnywhere, Category= "Player stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing= OnRep_Health, VisibleAnywhere,  Category="Player stats")
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health(float LastHealth);

	/*
	 * 玩家护盾
	 */
	UPROPERTY(EditAnywhere, Category= "Player stats")
	float MaxShield = 100.f;

	UPROPERTY(ReplicatedUsing= OnRep_Shield, EditAnywhere,  Category="Player stats")
	float Shield = 100.f;

	UFUNCTION()
	void OnRep_Shield(float LastShield);
	
	/*
	 * 玩家淘汰
	 */
	UPROPERTY()
	ABlasterPlayerController* BlasterPlayerController;
	
	bool bElimmed = false;
	FTimerHandle ElimTimer;
	
	UPROPERTY(EditDefaultsOnly)
	float ElimDelay  =3.f;
	
	void ElimTimeFinshed();

	// 退出游戏
	bool bLeftGame = false;
	
	// 融化效果
	UPROPERTY()
	UTimelineComponent* DissolveTimeline;
	FOnTimelineFloat DissolveTrack;
	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;
	
	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();

	// 改变材质的实例
	UPROPERTY(VisibleAnywhere, Category="Elim")
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	UPROPERTY(VisibleAnywhere, Category="Elim")
	UMaterialInstance* DissolveMaterialInstance;

	/*
	 * 团队角色颜色
	 */
	UPROPERTY(EditAnywhere, Category="Team")
	UMaterialInstance* RedDissolveMatInst;
	UPROPERTY(EditAnywhere, Category="Team")
	UMaterialInstance* RedMaterial;
	
	UPROPERTY(EditAnywhere, Category="Team")
	UMaterialInstance* BlueDissolveMatInst;
	UPROPERTY(EditAnywhere, Category="Team")
	UMaterialInstance* BlueMaterial;

	UPROPERTY(EditAnywhere, Category="Team")
	UMaterialInstance* OriginalMaterial;
	
	
	UPROPERTY()
	ABlasterPlayerState* BlasterPlayerState;
	UPROPERTY()
	ABlasterGameMode* BlasterGameMode;

	
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* CrownSystem;
	UPROPERTY()
	class UNiagaraComponent* CrownComponent;
	
	/*
	 * 手雷
	 */
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* GrenadeMesh;

	void UpdateHUDAmmo();

	/*
	 * 默认武器
	 */
	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass;
	
public:	
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();
	FORCEINLINE float GetAO_Yaw() const {return AO_Yaw;}
	FORCEINLINE float GetAO_Pitch() const {return AO_Pitch;}
	AWeapon* GetEquippedWeapon();
	
	FORCEINLINE ETurnInPlace GetTurningPlace() const {return TurningInPlace;}
	FVector GetHitTarget();
	
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return CameraComponent; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool GetElimmed() const { return bElimmed;}
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE void SetHealth(float HealAmount) { Health = HealAmount;}
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE void SetShield(float ShieldAmount) { Shield = ShieldAmount;}
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	ECombatState GetCombatStat() const;
	FORCEINLINE UCombatComponent* GetCombatComponet() const { return CombatComponent; }
	FORCEINLINE bool GetDisableGamplay() const { return bDisableGameplay; }
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }
	FORCEINLINE UStaticMeshComponent* GetGrenadeMesh() const { return GrenadeMesh; }
	FORCEINLINE UBuffComponent* GetBuffComponent() const { return BuffComponent; }
	FORCEINLINE bool ShouldSwap() const { return CombatComponent->EquippedWeapon && CombatComponent->SecondaryWeapon; }
	bool IsLocallyReloading();
	bool IsLocallySwapping();
	FORCEINLINE ULagCompensationComponent* GetLagCompensation() const { return LagCompensationComponent; }
};



