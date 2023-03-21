// Fill out your copyright notice in the Description page of Project Settings.


#include "ShotGunWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"


//之前实现的ShotGun，Scatter的生成产生于服务端
void AShotGunWeapon::Fire(const FVector& HitTarget)
{
	AWeapon::Fire(HitTarget);
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	//客户端在服务端中的SimulateProxy获取的Controller永远为空
	AController* InstigatorController = OwnerPawn->Controller;
	
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();

		TMap<ABlasterCharacter*, uint32> HitMap{};
		for (int32 i = 0; i < NumsOfPellet; i++)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			ABlasterCharacter* BlasterCharacter =  Cast<ABlasterCharacter>(FireHit.GetActor());
			if (BlasterCharacter && HasAuthority() && InstigatorController)
			{
				//利用哈希表存储击中不同角色的伤害次数
				if (HitMap.Contains(BlasterCharacter))
				{
					HitMap[BlasterCharacter]++;
				}
				else
				{
					HitMap.Emplace(BlasterCharacter, 1);
				}
			}
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				ImpactParticles,
				FireHit.ImpactPoint,
				FireHit.ImpactNormal.Rotation()
				);
			}
			if (HitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(
				this,
				HitSound,
				FireHit.ImpactPoint,
				.5f,
				FMath::FRandRange(-.5f, .5f)
				);
			}
		}

		for (auto HitPair : HitMap)
		{
			if (HitPair.Key && HasAuthority() && InstigatorController)
			{
				//扫描类武器只通过服务端产生伤害
				UGameplayStatics::ApplyDamage(
					HitPair.Key,
					Damage * HitPair.Value,
					InstigatorController,
					this,
					UDamageType::StaticClass()
					);
			}
		}
	}
}

//使用了Client-Side Rewind，将Scatter的功能抽离出来，放在了本地实现，然后通过传递HitTarget的Array实现服务端的判定
void AShotGunWeapon::FireShotGun(const TArray<FVector_NetQuantize>& HitTargetArray)
{
	AWeapon::Fire(FVector());
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	//客户端在服务端中的SimulateProxy获取的Controller永远为空
	AController* InstigatorController = OwnerPawn->Controller;

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();

		//霰弹枪一发子弹中的弹丸射击到玩家的次数
		TMap<ABlasterCharacter*, uint32> HitMap{};
		TMap<ABlasterCharacter*, uint32> HeadShotMap{};
 		for (auto HitTarget: HitTargetArray)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			ABlasterCharacter* BlasterCharacter =  Cast<ABlasterCharacter>(FireHit.GetActor());
			if (BlasterCharacter)
			{
				const bool bHeadShot = FireHit.BoneName.ToString() == FString("head");
				
				if (bHeadShot)
				{
					//利用哈希表存储击中不同角色的伤害次数
					if (HeadShotMap.Contains(BlasterCharacter)) HeadShotMap[BlasterCharacter]++;
					else HeadShotMap.Emplace(BlasterCharacter, 1);
				}
				else
				{
					//利用哈希表存储击中不同角色的伤害次数
					if (HitMap.Contains(BlasterCharacter)) HitMap[BlasterCharacter]++;
					else HitMap.Emplace(BlasterCharacter, 1);
				}
				
				if (ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticles,
					FireHit.ImpactPoint,
					FireHit.ImpactNormal.Rotation()
					);
				}
				if (HitSound)
				{
					UGameplayStatics::PlaySoundAtLocation(
					this,
					HitSound,
					FireHit.ImpactPoint,
					.5f,
					FMath::FRandRange(-.5f, .5f)
					);
				}
				
			}
		}

		TArray<ABlasterCharacter*> HitCharacters{};
		TMap<ABlasterCharacter*, float> DamageMap{};
		for (auto HitPair : HitMap)
		{
			if (HitPair.Key)
			{
				DamageMap.Emplace(HitPair.Key, HitPair.Value * Damage);
				HitCharacters.AddUnique(HitPair.Key);
			}
		}
		
		for (auto HeadShotHitPair : HeadShotMap)
		{
			if (HeadShotHitPair.Key)
			{
				if (DamageMap.Contains(HeadShotHitPair.Key)) DamageMap[HeadShotHitPair.Key] += HeadShotHitPair.Value * HeadShotDamage;
				else DamageMap.Emplace(HeadShotHitPair.Key, HeadShotHitPair.Value * HeadShotDamage);
				HitCharacters.AddUnique(HeadShotHitPair.Key);
			}
		}

		for (auto DamagePair: DamageMap)
		{
			if (DamagePair.Key && InstigatorController)
			{
				const bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
				if (HasAuthority() && bCauseAuthDamage)
				{
					//扫描类武器只通过服务端产生伤害
					UGameplayStatics::ApplyDamage(
						DamagePair.Key,
						DamagePair.Value,
						InstigatorController,
						this,
						UDamageType::StaticClass()
						);
				}
			}
		}
		


		//当本地客户端发射时，判断是否要使用ServerSide-Rewind
		if (!HasAuthority() && bUseServerSideRewind)
		{
			OwningCharacter = OwningCharacter == nullptr? Cast<ABlasterCharacter>(OwnerPawn): OwningCharacter;
			OwningPlayerController = OwningPlayerController == nullptr? Cast<ABlasterPlayerController>(InstigatorController):OwningPlayerController;
			if (OwningCharacter && OwningPlayerController && OwningCharacter->GetLagCompensation() && OwningCharacter->IsLocallyControlled())
			{
				OwningCharacter->GetLagCompensation()->ShotGunServerScoreRequest(
					HitCharacters,
					Start,
					HitTargetArray,
					OwningPlayerController->GetServerTime() - OwningPlayerController->SingleRoundTripTime
				);
			}
		}
	}
}

void AShotGunWeapon::ShotGunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargetArray)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return;
	
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector Start = SocketTransform.GetLocation();
	
	const FVector ToTargetNormalized = (HitTarget - Start).GetSafeNormal();
	const FVector SphereCenter = Start + ToTargetNormalized * DistanceToSphere;
	
	for (int32 i = 0; i < NumsOfPellet; i++)
	{
		const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		const FVector EndLoc = SphereCenter + RandVec;
		FVector ToEndLoc = EndLoc - Start;
		ToEndLoc = Start + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size();
		
		HitTargetArray.Add(ToEndLoc);
	}
}
