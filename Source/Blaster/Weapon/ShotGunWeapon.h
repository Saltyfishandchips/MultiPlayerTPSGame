// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"
#include "ShotGunWeapon.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AShotGunWeapon : public AHitScanWeapon
{
	GENERATED_BODY()

public:
	virtual void Fire(const FVector& HitTarget) override;
	virtual void FireShotGun(const TArray<FVector_NetQuantize>& HitTargetArray);

	void ShotGunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargetArray);
	
	
private:
	UPROPERTY(EditAnywhere)
	int32 NumsOfPellet = 10;
};
