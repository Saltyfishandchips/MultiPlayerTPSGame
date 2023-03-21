#pragma once

#define TRACE_LENGTH 10000.f

UENUM(BlueprintType)
enum class EWeaponType: uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),
	EWT_RocketLauncher UMETA(DisplayName = "Rocket Launcher"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	EWT_SMG UMETA(DisplayName = "SMG"),
	EWT_ShotGun UMETA(DisplayName = "ShotGun"),
	EWT_SniperRifle UMETA(DisplayName = "Sniper Rifle"),
	EWT_GrenadesLauncher UMETA(DisplayName = "GrenadesLauncher"),
	EWT_MAX UMETA(DisplayName = "DefaultMAX")
};
