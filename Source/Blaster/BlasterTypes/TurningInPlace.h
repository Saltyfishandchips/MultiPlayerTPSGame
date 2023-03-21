#pragma once

UENUM(BlueprintType)
enum class ETurnInPlace : uint8
{
	ETIP_Left UMETA(DisplayName = "Turing Left"),
	ETIP_Right UMETA(DisplayName = "Turing right"),
	ETIP_NoTurning UMETA(DisplayName = "Not Turning"),

	ETIP_MAX UMETA(DisplayName = "DefaultMAX")
};
