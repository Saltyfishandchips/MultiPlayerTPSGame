// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "DrawDebugHelpers.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	
	//客户端在服务端中的SimulateProxy获取的Controller永远为空
	AController* InstigatorController = OwnerPawn->Controller;
	
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);
		
		ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());

		if (BlasterCharacter  && InstigatorController)
		{
			const bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
			if (HasAuthority() && bCauseAuthDamage)
			{
				const float DamageToCause = FireHit.BoneName.ToString() == FString("head")? HeadShotDamage: Damage;
				
				//扫描类武器只通过服务端产生伤害
                UGameplayStatics::ApplyDamage(
                BlasterCharacter,
                DamageToCause,
                InstigatorController,
                this,
                UDamageType::StaticClass()
                );
			}
			else if (!HasAuthority() && bUseServerSideRewind)
			{
				OwningCharacter = OwningCharacter == nullptr? Cast<ABlasterCharacter>(OwnerPawn): OwningCharacter;
				OwningPlayerController = OwningPlayerController == nullptr? Cast<ABlasterPlayerController>(InstigatorController):OwningPlayerController;
				if (OwningCharacter && OwningPlayerController && OwningCharacter->GetLagCompensation() && OwningCharacter->IsLocallyControlled())
				{
					OwningCharacter->GetLagCompensation()->ServerScoreRequest(
						BlasterCharacter,
						Start,
						FireHit.ImpactPoint,
						OwningPlayerController->GetServerTime() - OwningPlayerController->SingleRoundTripTime
					);
				}
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
			FireHit.ImpactPoint
			);
		}
		
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				MuzzleFlash,
				SocketTransform);
		}
			
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				FireSound,
				GetActorLocation()
			);
		}
	}
}

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if (World)
	{
		
		// FVector End = bUseScatter? SetHitScatter(TraceStart, HitTarget) :TraceStart + (HitTarget - TraceStart) * 1.25f;
		FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;
		World->LineTraceSingleByChannel(
				OutHit,
				TraceStart,
				End,
				ECollisionChannel::ECC_Visibility
				);

		FVector BeamEnd = End;
		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}
		else
		{
			OutHit.ImpactPoint = End;
		}

		// DrawDebugSphere(GetWorld(), OutHit.ImpactPoint, 16.f, 12, FColor::Orange, true);
		if (BeamParticles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				World,
				BeamParticles,
				TraceStart,
				FRotator::ZeroRotator,
				true
			);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
		
	}
}



