// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"
#include "DrawDebugHelpers.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/Blaster.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	SaveFramePackage();
}

void ULagCompensationComponent::SaveFramePackage()
{
	if (Character == nullptr || !Character->HasAuthority()) return;
	if (FrameHistory.Num() <= 1)
	{
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
	}
	else
	{
		float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		while (HistoryLength > MaxRecordTime)
		{
			FrameHistory.RemoveNode(FrameHistory.GetTail());
			HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		}
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
		// ShowFramePackage(ThisFrame, FColor::Red);
	}
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	Character = Character == nullptr? Cast<ABlasterCharacter>(GetOwner()):Character;
	if (Character)
	{
		Package.Time = GetWorld()->GetTimeSeconds();
		Package.Character = Character;
		for (auto& BoxPair: Character->HitCollisionBoxMap)
		{
			FBoxInformation BoxInformation{};
			BoxInformation.Location = BoxPair.Value->GetComponentLocation();
			BoxInformation.Rotation = BoxPair.Value->GetComponentRotation();
			BoxInformation.BoxExtent = BoxPair.Value->GetScaledBoxExtent();

			Package.HitBoxInfo.Add(BoxPair.Key, BoxInformation);
		}
	}
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor Color)
{
	for (auto& BoxInfo: Package.HitBoxInfo)
	{
		DrawDebugBox(
			GetWorld(),
			BoxInfo.Value.Location,
			BoxInfo.Value.BoxExtent,
			FQuat(BoxInfo.Value.Rotation),
			Color,
			false,
			4.f);
	}
}

/**
 * Server Score Request
 */

void ULagCompensationComponent::ProjectileServerScoreRequest_Implementation(ABlasterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	FServerSideRewindResult Confirm = ProjectileServerSideRewind(HitCharacter, TraceStart, InitialVelocity, HitTime);
	if (Character && HitCharacter && Confirm.bHitConfirmed && Character->GetEquippedWeapon())
	{
		const float Damage = Confirm.bHeadShot? Character->GetEquippedWeapon()->GetHeadShotDamage(): Character->GetEquippedWeapon()->GetDamage();
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			Damage,
			Character->Controller,
			Character->GetEquippedWeapon(),
			UDamageType::StaticClass()
			);
	}
}


void ULagCompensationComponent::ServerScoreRequest_Implementation(ABlasterCharacter* HitCharacter,
const FVector_NetQuantize& TraceStart,const FVector_NetQuantize& HitLocation,float HitTime)
{
	FServerSideRewindResult Confirm = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);
	if (Character && HitCharacter && Confirm.bHitConfirmed && Character->GetEquippedWeapon())
	{

		const float Damage = Confirm.bHeadShot? Character->GetEquippedWeapon()->GetHeadShotDamage(): Character->GetEquippedWeapon()->GetDamage();
		
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			Damage,
			Character->Controller,
			Character->GetEquippedWeapon(),
			UDamageType::StaticClass()
			);
	}
}

void ULagCompensationComponent::ShotGunServerScoreRequest_Implementation(
	const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	FShotGunServerSideRewindResult Confirm = ShotGunServerSideRewind(HitCharacters, TraceStart, HitLocations, HitTime);

	for (auto& HitCharacter: HitCharacters)
	{
		if (HitCharacter == nullptr || HitCharacter->GetEquippedWeapon() == nullptr) continue;
		float TotalDamage = 0.f;
		if (Confirm.HeadShots.Contains(HitCharacter))
		{
			const float HeadShotDamage = Confirm.HeadShots[HitCharacter] * HitCharacter->GetEquippedWeapon()->GetHeadShotDamage();
			TotalDamage += HeadShotDamage;
		}
		if (Confirm.BodyShots.Contains(HitCharacter))
		{
			const float BodyShotDamage = Confirm.BodyShots[HitCharacter] * HitCharacter->GetEquippedWeapon()->GetDamage();
			TotalDamage += BodyShotDamage;
		}
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			TotalDamage,
			Character->Controller,
			Character->GetEquippedWeapon(),
			UDamageType::StaticClass()
		);
	}
}

/*
 * Projectile SSR
 */
FServerSideRewindResult ULagCompensationComponent::ProjectileServerSideRewind(ABlasterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	const FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);	
	return ProjectileConfirmHit(FrameToCheck, HitCharacter, TraceStart, InitialVelocity, HitTime);
}

/*
 * HitScan SSR
 */
FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
                                                                    const FVector_NetQuantize& HitLocation, float HitTime)
{
	const FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);	
	return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
}

/*
 * ShotGun SSR
 */
FShotGunServerSideRewindResult ULagCompensationComponent::ShotGunServerSideRewind(
	const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	
	TArray<FFramePackage> FrameToCheck;
	for (ABlasterCharacter* HitCharacter:HitCharacters)
	{
		FrameToCheck.Add(GetFrameToCheck(HitCharacter, HitTime));
	}
	return ShotGunConfirmHit(FrameToCheck, TraceStart, HitLocations);
}

/**
 * Confirm Hit
 */
// Projectile ConfirmHit
FServerSideRewindResult ULagCompensationComponent::ProjectileConfirmHit(const FFramePackage& Package,
	ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	FFramePackage CurrentFrame;
	//将原先HitCharacter的信息保存下来
	CacheBoxPositions(HitCharacter, CurrentFrame);
	MoveBoxes(HitCharacter, Package);

	//Server-Rewind的时候关闭Mesh的碰撞，以避免对Rewind结果产生影响
	EnableHitCharacterMesh(HitCharacter, ECollisionEnabled::NoCollision);
	
	// 首先开启头部的Hit Box
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxMap[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);

	FPredictProjectilePathParams PathParams;
	PathParams.bTraceWithCollision = true;
	PathParams.MaxSimTime = MaxRecordTime;
	PathParams.LaunchVelocity = InitialVelocity;
	PathParams.StartLocation = TraceStart;
	PathParams.SimFrequency = 15.f;
	PathParams.ProjectileRadius = 5.f;
	PathParams.TraceChannel = ECC_HitBox;
	PathParams.ActorsToIgnore.Add(GetOwner());
	PathParams.DrawDebugTime = 5.f;
	PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;
	
	FPredictProjectilePathResult PathResult;
	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);

	if (PathResult.HitResult.bBlockingHit)
	{
		//击中头部，可以直接返回
		ResetBoxes(HitCharacter, CurrentFrame);
		EnableHitCharacterMesh(HitCharacter, ECollisionEnabled::QueryAndPhysics);
		//
		HitCharacter->GetMesh()->SetCollisionResponseToChannel(ECC_HitBox,ECollisionResponse::ECR_Block);
		return FServerSideRewindResult{true, true};
	}
	else
	{
		//没有HeadShot，检测剩余的HitBox
		for (auto& HitBoxPair: HitCharacter->HitCollisionBoxMap)
		{
			if (HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			}
		}
		UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
		if (PathResult.HitResult.bBlockingHit)
		{
			//击中身体，可以直接返回
			ResetBoxes(HitCharacter, CurrentFrame);
			EnableHitCharacterMesh(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{true, false};
		}
	}
	//没有击中，可以直接返回
	ResetBoxes(HitCharacter, CurrentFrame);
	EnableHitCharacterMesh(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{false, false};
}

// HitScan ConfirmHit
FServerSideRewindResult ULagCompensationComponent::ConfirmHit(const FFramePackage& Package,
	ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	if (HitCharacter == nullptr) return FServerSideRewindResult();
	
	FFramePackage CurrentFrame;
	//将原先HitCharacter的信息保存下来
	CacheBoxPositions(HitCharacter, CurrentFrame);
	MoveBoxes(HitCharacter, Package);

	//Server-Rewind的时候关闭Mesh的碰撞，以避免对Rewind结果产生影响
	EnableHitCharacterMesh(HitCharacter, ECollisionEnabled::NoCollision);
	
	// 首先开启头部的Hit Box
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxMap[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);

	FHitResult ConfirmHitResult;
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
	UWorld* World = GetWorld();
	if (World)
	{
		World->LineTraceSingleByChannel(
			ConfirmHitResult,
			TraceStart,
			TraceEnd,
			ECC_HitBox
			);
		if (ConfirmHitResult.bBlockingHit)
		{
			//击中头部，可以直接返回
			ResetBoxes(HitCharacter, CurrentFrame);
			EnableHitCharacterMesh(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			//
			HitCharacter->GetMesh()->SetCollisionResponseToChannel(ECC_HitBox,ECollisionResponse::ECR_Block);
			return FServerSideRewindResult{true, true};
		}
		else
		{
			//没有HeadShot，检测剩余的HitBox
			for (auto& HitBoxPair: HitCharacter->HitCollisionBoxMap)
			{
				if (HitBoxPair.Value != nullptr)
				{
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
				}
			}
			World->LineTraceSingleByChannel(
					ConfirmHitResult,
					TraceStart, TraceEnd,
					ECC_HitBox
					);
			if (ConfirmHitResult.bBlockingHit)
			{
				//击中身体，可以直接返回
				ResetBoxes(HitCharacter, CurrentFrame);
				EnableHitCharacterMesh(HitCharacter, ECollisionEnabled::QueryAndPhysics);
				return FServerSideRewindResult{true, false};
			}
		}
	}
	//没有击中，可以直接返回
	ResetBoxes(HitCharacter, CurrentFrame);
	EnableHitCharacterMesh(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{false, false};
}

// ShotGun ConfirmHit
FShotGunServerSideRewindResult ULagCompensationComponent::ShotGunConfirmHit(const TArray<FFramePackage>& FramePackage, const FVector_NetQuantize& TraceStart,const TArray<FVector_NetQuantize>& HitLocation)
{
	for (auto& Frame: FramePackage)
	{
		if (Frame.Character == nullptr) return FShotGunServerSideRewindResult();
	}
	
	FShotGunServerSideRewindResult ShotGunResult;
	TArray<FFramePackage> CurrentFrames;
	for (auto& Frame: FramePackage)
	{
		FFramePackage CurrentFrame;
		
		//将原先HitCharacter的信息保存下来
		CurrentFrame.Character = Frame.Character;
		CacheBoxPositions(Frame.Character, CurrentFrame);
		MoveBoxes(Frame.Character, Frame);
		//Server-Rewind的时候关闭Mesh的碰撞，以避免对Rewind结果产生影响
		EnableHitCharacterMesh(Frame.Character, ECollisionEnabled::NoCollision);
		CurrentFrames.Add(CurrentFrame);
	}

	for (auto& Frame: FramePackage)
	{
		// 首先开启头部的Hit Box
		UBoxComponent* HeadBox = Frame.Character->HitCollisionBoxMap[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
	}

	UWorld* World = GetWorld();
	//检测HeadShot
	for (auto& Location: HitLocation)
	{
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (Location - TraceStart) * 1.25f;
		if (World)
		{
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_HitBox
				);
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(ConfirmHitResult.GetActor());
			if (BlasterCharacter)
			{
				if (ShotGunResult.HeadShots.Contains(BlasterCharacter))
				{
					ShotGunResult.HeadShots[BlasterCharacter]++;
				}
				else
				{
					ShotGunResult.HeadShots.Emplace(BlasterCharacter, 1);
				}
			}
		}
	}

	// 启用每个弹丸Hit Box的碰撞，然后关闭头部Box碰撞
	for (auto& Frame: FramePackage)
	{
		//没有HeadShot，检测剩余的HitBox
		for (auto& HitBoxPair: Frame.Character->HitCollisionBoxMap)
		{
			if (HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			}
		}
		UBoxComponent* HeadBox = Frame.Character->HitCollisionBoxMap[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	//检测BodyShot
	for (auto& Location: HitLocation)
	{
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (Location - TraceStart) * 1.25f;
		if (World)
		{
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_HitBox
				);
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(ConfirmHitResult.GetActor());
			if (BlasterCharacter)
			{
				if (ShotGunResult.BodyShots.Contains(BlasterCharacter))
				{
					ShotGunResult.BodyShots[BlasterCharacter]++;
				}
				else
				{
					ShotGunResult.BodyShots.Emplace(BlasterCharacter, 1);
				}
			}
		}
	}

	//没有击中，可以直接返回
	for (auto& Frame: CurrentFrames)
	{
		ResetBoxes(Frame.Character, Frame);
		EnableHitCharacterMesh(Frame.Character, ECollisionEnabled::QueryAndPhysics);
	}

	return ShotGunResult;
}

FFramePackage ULagCompensationComponent::GetFrameToCheck(ABlasterCharacter* HitCharacter, float HitTime)
{
	const bool bReturn =
		HitCharacter == nullptr || 
		HitCharacter->GetLagCompensation() == nullptr ||
		HitCharacter->GetLagCompensation()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagCompensation()->FrameHistory.GetTail() == nullptr;
	if (bReturn) return FFramePackage();
	
	FFramePackage FrameToCheck{};
	
	bool bShouldInterpolate = true;
	//被击中角色的帧历史，本地引用
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensation()->FrameHistory;

	const float OldestHistoryTime = History.GetTail()->GetValue().Time;
	const float NewestHistoryTime = History.GetHead()->GetValue().Time;
	if (OldestHistoryTime > HitTime)
	{
		//击中事件太久，帧历史中没有保存
		return FFramePackage();
	}
	if (OldestHistoryTime == HitTime)
	{
		FrameToCheck = History.GetTail()->GetValue();
		bShouldInterpolate = false;
	}
	if (NewestHistoryTime <= HitTime)
	{
		//HitTime在保存历史帧最新时间的之后，则使用Head()
		FrameToCheck = History.GetHead()->GetValue();
		bShouldInterpolate = false;
	}

	// 遍历双向链表
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = History.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Newer = Older;
	while (Older->GetValue().Time > HitTime)
	{
		if (Older->GetNextNode() == nullptr) return FFramePackage();
		Older = Older->GetNextNode();
		if (Older->GetValue().Time > HitTime)
		{
			Newer = Older;
		}
	}

	//储存的中间帧时间等于命中时间（基本不太可能）
	if (Older->GetValue().Time == HitTime)
	{
		FrameToCheck = Older->GetValue();
		bShouldInterpolate = false;
	}
	
	if (bShouldInterpolate)
	{
		//寻找到了击中时间的前一帧和后一帧，我们使用插值
		FrameToCheck = InterpBetweenFrames(Older->GetValue(), Newer->GetValue(), HitTime);
		FrameToCheck.Time = HitTime;
	}

	FrameToCheck.Character = HitCharacter;
	return FrameToCheck;
}

//击中帧的插值算法
FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame,
	const FFramePackage& NewerFrame, float HitTime)
{
	const float Distance = NewerFrame.Time - OlderFrame.Time;
	const float ScoreFactor = FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0.f, 1.f);
	// const float ScoreFactor = (HitTime - OlderFrame.Time) / Distance;
	
	FFramePackage InterpPackage{};
	InterpPackage.Time = HitTime;
	for (auto& Package: OlderFrame.HitBoxInfo)
	{
		const FName& BoxName = Package.Key;
		const FBoxInformation& OlderBox = OlderFrame.HitBoxInfo[BoxName];
		const FBoxInformation& NewerBox = NewerFrame.HitBoxInfo[BoxName];
	
		FBoxInformation InterpBoxInfo;
		InterpBoxInfo.Location = FMath::VInterpTo(OlderBox.Location, NewerBox.Location, 1.f, ScoreFactor);
		InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBox.Rotation, NewerBox.Rotation, 1.f, ScoreFactor);
		InterpBoxInfo.BoxExtent = OlderBox.BoxExtent;
	
		InterpPackage.HitBoxInfo.Add(BoxName, InterpBoxInfo);
	}
	
	return InterpPackage;
}

void ULagCompensationComponent::MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr) return;

	for (auto& HitBoxPair: HitCharacter->HitCollisionBoxMap)
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
		}
	}
}

void ULagCompensationComponent::ResetBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr) return;

	for (auto& HitBoxPair: HitCharacter->HitCollisionBoxMap)
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
			//恢复原先的HitBox信息，同时将碰撞改为无碰撞
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void ULagCompensationComponent::CacheBoxPositions(ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if (HitCharacter == nullptr) return;
	for (auto& HitBoxPair: HitCharacter->HitCollisionBoxMap)
	{
		if (HitBoxPair.Value != nullptr)
		{
			FBoxInformation BoxInfo;
			BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();

			OutFramePackage.HitBoxInfo.Add(HitBoxPair.Key, BoxInfo);
		}
	}
}

void ULagCompensationComponent::EnableHitCharacterMesh(ABlasterCharacter* HitCharacter,
	ECollisionEnabled::Type ColllisionEnable)
{
	if (Character && Character->GetMesh())
	{
		Character->GetMesh()->SetCollisionEnabled(ColllisionEnable);
	}
}

