// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	HealRampUp(DeltaTime);
	ShieldRamUp(DeltaTime);
}

void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
	bHealing = true;
	HealingRate = HealAmount / HealingTime;
	AmountToHeal += HealAmount;
}

void UBuffComponent::HealRampUp(float DeltaTime)
{
	if (bHealing == false || Character == nullptr || Character->GetElimmed()) return;

	const float HealThisFrame = HealingRate * DeltaTime;
	Character->SetHealth(FMath::Clamp(Character->GetHealth() + HealThisFrame, 0.f, Character->GetMaxHealth()));
	Character->UpdateHUDHealth();
	AmountToHeal -= HealThisFrame;

	if (AmountToHeal < 0 || Character->GetHealth() >= Character->GetMaxHealth())
	{
		bHealing = false;
		AmountToHeal = 0;
	}
}

void UBuffComponent::ReChargeShield(float ShieldAmount, float DurationTime)
{
	bRecharging = true;
	ShieldRate = ShieldAmount / DurationTime;
	AmountToShield += ShieldAmount;
}

void UBuffComponent::ShieldRamUp(float DeltaTime)
{
	if (bRecharging == false || Character == nullptr || Character->GetElimmed()) return;

	const float RechargeThisFrame = ShieldRate * DeltaTime;
	Character->SetShield(FMath::Clamp(Character->GetShield() + RechargeThisFrame, 0.f, Character->GetMaxShield()));
	Character->UpdateHUDShield();
	AmountToShield -= RechargeThisFrame;

	if (AmountToHeal < 0 || Character->GetShield() >= Character->GetMaxShield())
	{
		bRecharging = false;
		AmountToShield = 0;
	}
}

void UBuffComponent::SetInitialSpeed(float WalkSpeed, float CrouchSpeed)
{
	InitialWalkSpeed = WalkSpeed;
	InitialCrouchSpeed = CrouchSpeed;
}

void UBuffComponent::Speedup(float WalkSpeedup, float CrouchSpeedup, float Time)
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;
	
	Character->GetWorldTimerManager().SetTimer(
		SpeedupTimerHandle,
		this,
		&UBuffComponent::ResetSpeed,
		Time
	);
	Character->GetCharacterMovement()->MaxWalkSpeed = WalkSpeedup;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeedup;
	Multicast_Speedup(WalkSpeedup, CrouchSpeedup);
}

void UBuffComponent::Multicast_Speedup_Implementation(float WalkSpeedup, float CrouchSpeedup)
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;
	Character->GetCharacterMovement()->MaxWalkSpeed = WalkSpeedup;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeedup;
}

void UBuffComponent::ResetSpeed()
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;
	Character->GetCharacterMovement()->MaxWalkSpeed = InitialWalkSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchSpeed;
	Multicast_ResetSpeed();
}

void UBuffComponent::Multicast_ResetSpeed_Implementation()
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;
	Character->GetCharacterMovement()->MaxWalkSpeed = InitialWalkSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchSpeed;
}