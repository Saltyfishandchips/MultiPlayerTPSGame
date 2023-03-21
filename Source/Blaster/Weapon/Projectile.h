// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UBoxComponent;
class UProjectileMovementComponent;
class UParticleSystem;
class UParticleSystemComponent;
class USoundCue;

UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void Tick(float DeltaTime) override;
	//这个函数在Actor游玩被销毁时调用，并且由于Projectile是Replicate的，这个函数不仅在服务端，也会在客户端中调用
	//因此我们可以利用这个函数，将播放粒子效果和声效都放在这个override函数内，以减少RPC的使用，降低网络带宽，提高游戏性能。
	virtual void Destroyed() override;

	bool bUseServerSideRewind = false;
	FVector_NetQuantize TraceStart;
	FVector_NetQuantize100 InitialVelocity;

	UPROPERTY(EditAnywhere)
	float InitialSpeed = 15000;

	//只对火箭弹和榴弹有效
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	//与火箭弹与榴弹无关
	UPROPERTY(EditAnywhere)
	float HeadShotDamage = 40.f;
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);


	void StartDestroyedTimer();
	void DestroyedTimerFinished();
	

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailSystem;
	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;
	void SpawnTrailSystem();

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;

	//击中的粒子效果和声效
	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	USoundCue* ImpactSound;

	UPROPERTY(EditAnywhere)
	UBoxComponent* CollisionBox;
	
	UPROPERTY(VisibleAnywhere)
    UProjectileMovementComponent* ProjectileMovementComponent;

	void ApplyRadiusDamage();
	
private:
	
	UPROPERTY(EditAnywhere)
	UParticleSystem* Tracer;

	UPROPERTY(EditAnywhere)
	UParticleSystemComponent* TracerComponent;

	FTimerHandle DestroyTimer;
	UPROPERTY(EditAnywhere)
	float DestroyDelay = 3.f;

	/*
	 *	抛射物的径向伤害设置
	 */
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float MinDamage = 20.f;

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float DamageInterRadius = 200.f;

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float DamageOuterRadius = 800.f;

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float DamageFallOff = 2.f;
	
public:	
};
