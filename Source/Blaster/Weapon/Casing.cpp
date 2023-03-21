

#include "Casing.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

ACasing::ACasing()
{
 	
	PrimaryActorTick.bCanEverTick = false;

	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(CasingMesh);

	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	CasingMesh->SetSimulatePhysics(true);
	CasingMesh->SetEnableGravity(true);
	CasingMesh->SetNotifyRigidBodyCollision(true);
	
	ShellEjectionImpulse = 10.f;
	ShellExistTime = 2.f;
	bSoundPlay = false;
}


void ACasing::BeginPlay()
{
	Super::BeginPlay();

	CasingMesh->OnComponentHit.AddDynamic(this, &ACasing::OnHit);
	CasingMesh->AddImpulse(GetActorForwardVector() * ShellEjectionImpulse);

	FTimerHandle ShellTimer;
	GetWorldTimerManager().SetTimer(ShellTimer, this, &ACasing::ShellDestroy, ShellExistTime);
	
}

void ACasing::ShellDestroy()
{
	Destroy();
}

void ACasing::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (ShellSound && !bSoundPlay)
	{
		bSoundPlay = true;
		UGameplayStatics::PlaySoundAtLocation(this, ShellSound,  GetActorLocation());
	}


}



