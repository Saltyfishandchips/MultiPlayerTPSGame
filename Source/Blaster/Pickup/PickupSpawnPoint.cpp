// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupSpawnPoint.h"
#include "Pickup.h"

APickupSpawnPoint::APickupSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void APickupSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		StartSpawnTimer((AActor*)nullptr);
	}
	
}

void APickupSpawnPoint::SpawnPickup()
{
	const int32 SelectionNum = FMath::RandRange(0, SpawnArray.Num() - 1);
	UWorld* World = GetWorld();
	if (World)
	{
		SpawnedPickup = World->SpawnActor<APickup>(SpawnArray[SelectionNum], GetActorTransform());
	}
	if (HasAuthority() && SpawnedPickup)
	{
		SpawnedPickup->OnDestroyed.AddDynamic(this, &ThisClass::StartSpawnTimer);
	}
}

void APickupSpawnPoint::StartSpawnTimer(AActor* DestroyedActor)
{
	float SpawnDelay = FMath::FRandRange(MinSpawnTime, MaxSpawnTime);
	GetWorldTimerManager().SetTimer(SpawnTimerHandle,
		this,
		&ThisClass::SpawnTimerFinished,
		SpawnDelay);
}

void APickupSpawnPoint::SpawnTimerFinished()
{
	if (HasAuthority())
	{
		SpawnPickup();
	}
}

