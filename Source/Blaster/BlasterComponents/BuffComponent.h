// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuffComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void Heal(float HealAmount, float HealingTime);
	void ReChargeShield(float ShieldAmount, float DurationTime);
	
	void Speedup(float WalkSpeedup, float CrouchSpeedup, float Time);
	//设置和恢复初始速度
	void SetInitialSpeed(float WalkSpeed, float CrouchSpeed);
protected:
	virtual void BeginPlay() override;
	void HealRampUp(float DeltaTime);

	void ShieldRamUp(float DeltaTime);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Speedup(float WalkSpeedup, float CrouchSpeedup);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ResetSpeed();
	
private:
	friend class ABlasterCharacter;
	UPROPERTY()
	ABlasterCharacter* Character;

	// 回复血量
	bool bHealing = false;
	float HealingRate = 0;
	float AmountToHeal = 0.f;

	// 回复护盾
	bool bRecharging = false;
	float ShieldRate = 0;
	float AmountToShield = 0.f;

	// 加速
	FTimerHandle SpeedupTimerHandle;
	void ResetSpeed();
	float InitialWalkSpeed = 0;
	float InitialCrouchSpeed = 0;
public:	
	
};
