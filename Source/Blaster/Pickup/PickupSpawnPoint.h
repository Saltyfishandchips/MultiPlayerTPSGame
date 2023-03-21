// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupSpawnPoint.generated.h"

UCLASS()
class BLASTER_API APickupSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	APickupSpawnPoint();

protected:
	virtual void BeginPlay() override;

private:

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class APickup>> SpawnArray;

	UPROPERTY(EditAnywhere)
	float MinSpawnTime;
	UPROPERTY(EditAnywhere)
	float MaxSpawnTime;
	
	void SpawnPickup();

	UPROPERTY()
	APickup* SpawnedPickup;

	UFUNCTION()
	void StartSpawnTimer(AActor* DestroyedActor);
	FTimerHandle SpawnTimerHandle;
	void SpawnTimerFinished();
	
public:	

};
