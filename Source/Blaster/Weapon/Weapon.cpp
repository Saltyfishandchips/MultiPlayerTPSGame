// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Casing.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/KismetMathLibrary.h"


AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);
	
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetupAttachment(RootComponent);
	SphereComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);

	OutLineMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OutLineMesh"));
	OutLineMesh->SetupAttachment(RootComponent);
	OutLineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OutLineMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	/*
	 * 由于在Blaster中，我们将EquipWeapon也检查了HasAuthority()，因此对于下面这些装饰性的功能，可以省去检查
	 */
	// if (HasAuthority()) 
	// {
	// 	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	// 	SphereComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	// 	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
	// 	SphereComponent->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnSphereEndOverlap);
	// }
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SphereComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
	SphereComponent->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnSphereEndOverlap);
	
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		BlasterCharacter->SetOverlappingWeapon(this);
	}
	
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		BlasterCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	SetHUDAmmo();
	if (HasAuthority())
	{
		ClientUpdateAmmo(Ammo);
	}
	else
	{
		Sequence++;
	}
}

void AWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	if (HasAuthority()) return;
	Sequence--;
	Ammo = ServerAmmo - Sequence;
	SetHUDAmmo();
}

//只会在服务端调用(Component->UpdateAmmoValue();)
void AWeapon::AddAmmo(const int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
	ClientAddAmmo(AmmoToAdd);
}

//只在客户端调用，不会在聆听服务器上调用
void AWeapon::ClientAddAmmo_Implementation(int32 AmmoToAdd)
{
	if (HasAuthority()) return;
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	OwningCharacter = OwningCharacter == nullptr? Cast<ABlasterCharacter>(GetOwner()): OwningCharacter;
	if (OwningCharacter && OwningCharacter->GetCombatComponet() && IsFull())
	{
		OwningCharacter->GetCombatComponet()->JumpToShotGunEnd();
	}
	SetHUDAmmo();
}

// void AWeapon::OnRep_Ammo()
// {
// 	OwningCharacter == nullptr? Cast<ABlasterCharacter>(GetOwner()): OwningCharacter;
// 	if (OwningCharacter && OwningCharacter->GetCombatComponet() && IsFull())
// 	{
// 		OwningCharacter->GetCombatComponet()->JumpToShotGunEnd();
// 	}
// 	SetHUDAmmo();
// }

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	if (GetOwner() == nullptr)
	{
		OwningCharacter = nullptr;
		OwningPlayerController = nullptr;
	}
	else
	{
		OwningCharacter = OwningCharacter == nullptr? Cast<ABlasterCharacter>(GetOwner()): OwningCharacter;
		if (OwningCharacter && OwningCharacter->GetEquippedWeapon() && OwningCharacter->GetEquippedWeapon() == this)
		{
			SetHUDAmmo();
		}
	}
}

void AWeapon::SetHUDAmmo()
{
	OwningCharacter = OwningCharacter == nullptr? Cast<ABlasterCharacter>(GetOwner()): OwningCharacter;
	if (OwningCharacter)
	{
		OwningPlayerController = OwningPlayerController == nullptr? Cast<ABlasterPlayerController>(OwningCharacter->Controller): OwningPlayerController;
		if (OwningPlayerController)
		{
			OwningPlayerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

void AWeapon::OnWeaponStateSet()
{
	//装备上后，将拾取Widget显示关闭
	switch (WeaponState)
	{
	case EWeaponState::EWS_Initial:
		break;
	case EWeaponState::EWS_Equipped:
		OnEquipped();
		break;
	case EWeaponState::EWS_EquippedSecondary:
		OnEquippedSecondary();
		break;
	case EWeaponState::EWS_Dropped:
		OnDropped();
		break;
	case EWeaponState::EWS_MAX:
		break;
	}
}

void AWeapon::OnEquipped()
{
	ShowPickupWidget(false);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	OutLineMesh->SetVisibility(false);
	//拾起SMG时启用背带的物理效果
	if (WeaponType == EWeaponType::EWT_SMG)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}

	OwningCharacter = OwningCharacter == nullptr? Cast<ABlasterCharacter>(GetOwner()): OwningCharacter;
	if (OwningCharacter && bUseServerSideRewind)
	{
		OwningPlayerController = OwningPlayerController == nullptr? Cast<ABlasterPlayerController>(OwningCharacter->Controller): OwningPlayerController;
		if (OwningPlayerController && HasAuthority() && !OwningPlayerController->HighPingDelegate.IsBound())
		{
			OwningPlayerController->HighPingDelegate.AddDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

void AWeapon::OnEquippedSecondary()
{
	ShowPickupWidget(false);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	OutLineMesh->SetVisibility(false);
	//拾起SMG时启用背带的物理效果
	if (WeaponType == EWeaponType::EWT_SMG)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}

	OwningCharacter = OwningCharacter == nullptr? Cast<ABlasterCharacter>(GetOwner()): OwningCharacter;
	if (OwningCharacter && bUseServerSideRewind)
	{
		OwningPlayerController = OwningPlayerController == nullptr? Cast<ABlasterPlayerController>(OwningCharacter->Controller): OwningPlayerController;
		if (OwningPlayerController && HasAuthority() && OwningPlayerController->HighPingDelegate.IsBound())
		{
			OwningPlayerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

void AWeapon::OnDropped()
{
	if (HasAuthority())
	{
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	GetWeaponMesh()->SetSimulatePhysics(true);
	GetWeaponMesh()->SetEnableGravity(true);
	GetWeaponMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	OutLineMesh->SetVisibility(true);

	OwningCharacter = OwningCharacter == nullptr? Cast<ABlasterCharacter>(GetOwner()): OwningCharacter;
	if (OwningCharacter && bUseServerSideRewind)
	{
		OwningPlayerController = OwningPlayerController == nullptr? Cast<ABlasterPlayerController>(OwningCharacter->Controller): OwningPlayerController;
		if (OwningPlayerController && HasAuthority() && OwningPlayerController->HighPingDelegate.IsBound())
		{
			OwningPlayerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}


void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	OnWeaponStateSet();
}

void AWeapon::OnPingTooHigh(bool bPingTooHigh)
{
	bUseServerSideRewind = !bPingTooHigh;
}

void AWeapon::OnRep_WeaponState()
{
	OnWeaponStateSet();
}

void AWeapon::ShowPickupWidget(const bool bShowWidget) const
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}
	if (CasingClass)
	{
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));
		if (AmmoEjectSocket)
		{
			const FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);
			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<ACasing>(
					CasingClass,
					SocketTransform.GetLocation(),
					SocketTransform.GetRotation().Rotator()
				);
			}
		}
	}
	//我们使用Client-Side Prediction
	SpendRound();
	// if (HasAuthority())
	// {
	// 	SpendRound();	
	// }
	
}

FVector AWeapon::SetHitScatter(const FVector& HitTarget) const
{

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return FVector(); 
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector Start = SocketTransform.GetLocation();
	
	const FVector ToTargetNormalized = (HitTarget - Start).GetSafeNormal();
	const FVector SphereCenter = Start + ToTargetNormalized * DistanceToSphere;
	const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	const FVector EndLoc = SphereCenter + RandVec;
	const FVector ToEndLoc = EndLoc - Start;

	return FVector(Start + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());
}



void AWeapon::Drop()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	const FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	GetWeaponMesh()->DetachFromComponent(DetachRules);
	if (bShouldDestroyed)
	{
		Destroy();
	}
	
	SetOwner(nullptr);
	//丢弃的时候需要将Weapon获取的Character和Controller清零。
	OwningPlayerController = nullptr;
	OwningCharacter = nullptr;
}


bool AWeapon::IsEmpty()
{
	return Ammo <= 0;
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME_CONDITION(AWeapon, bUseServerSideRewind, COND_OwnerOnly);
	// DOREPLIFETIME(AWeapon, Ammo);
}
