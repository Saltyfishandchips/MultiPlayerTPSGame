#pragma once

UENUM(BlueprintType)
enum class ECombatState: uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccuiped"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),
	ECS_ThrowGrenade UMETA(DisplayName = "Throwing Grendad"),
	ECS_SwapWeapon UMETA(DisplayName = "Swap Weapon"),
	ECS_MAX UMETA(DisplayName = "DefaultMAX")
};
