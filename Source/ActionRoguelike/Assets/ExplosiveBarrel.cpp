

#include "ExplosiveBarrel.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "TimerManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Kismet/GameplayStatics.h"




AExplosiveBarrel::AExplosiveBarrel()
{	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));;
	RootComponent = StaticMeshComponent;
	StaticMeshComponent->SetCollisionProfileName("PhysicsActor");
	StaticMeshComponent->SetSimulatePhysics(true);
	
	LoopedNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LoopedNiagaraComp"));
	LoopedNiagaraComponent->SetupAttachment(StaticMeshComponent);
	LoopedNiagaraComponent->bAutoActivate = false;

	LoopedAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("LoopedAudioComponent"));
	LoopedAudioComponent->SetupAttachment(StaticMeshComponent);
	LoopedAudioComponent->bAutoActivate = false;

	RadialForceComponent = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
	RadialForceComponent->SetupAttachment(StaticMeshComponent);
	RadialForceComponent->Radius = 750.0f;
	RadialForceComponent->ImpulseStrength = 2500.0f;
	RadialForceComponent->bImpulseVelChange = true;    // ignore mass for consistent knockback
	RadialForceComponent->bIgnoreOwningActor = false;   // don't shove the barrel itself
	RadialForceComponent->AddCollisionChannelToAffect(ECC_PhysicsBody);
	RadialForceComponent->AddCollisionChannelToAffect(ECC_WorldDynamic);
	
}

void AExplosiveBarrel::OnActorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
								  FVector NormalImpulse, const FHitResult& Hit)
{
	// Only ignite once, even if the barrel is hit again before it explodes.
	if (bIgnited || GetWorldTimerManager().TimerExists(ExplosionTimerHandle))
	{
		return;
	}
	bIgnited = true;

	FVector HitFromDirection = GetActorRotation().Vector();

	UGameplayStatics::ApplyPointDamage
	(OtherActor, 10.f, HitFromDirection, Hit, GetInstigatorController(), this, DmgTypeClass);

	// Assign the ignition assets to the pre-created looped components and start them.
	LoopedNiagaraComponent->SetAsset(IgnitionEffect);
	LoopedNiagaraComponent->Activate(true);

	LoopedAudioComponent->SetSound(IgnitionSound);
	LoopedAudioComponent->Play();

	// Schedule the single explosion; ExplosionTimerElapsed will fire once after this delay.
	
	GetWorldTimerManager().SetTimer
	(ExplosionTimerHandle, this, &AExplosiveBarrel::Explode, ExplosionDelayTime);
}

void AExplosiveBarrel::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	StaticMeshComponent->OnComponentHit.AddDynamic(this, &AExplosiveBarrel::OnActorHit);
}

void AExplosiveBarrel::Explode()
{
	// Stop the looped ignition effects that were started in OnActorHit.
	if (LoopedNiagaraComponent)
	{
		LoopedNiagaraComponent->Deactivate();
	}
	if (LoopedAudioComponent)
	{
		LoopedAudioComponent->Stop();
	}

	// The explosion itself — fires only once (timer is non-repeating and is not re-set here).
	UNiagaraFunctionLibrary::SpawnSystemAttached
	(ExplosionEffect, StaticMeshComponent, NAME_None,
	FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);

	StaticMeshComponent->AddImpulse(FVector::UpVector * 1000, NAME_None, true);
	StaticMeshComponent->AddAngularImpulseInDegrees(FVector::RightVector * 1000, NAME_None, true);
	UGameplayStatics::PlaySound2D(this, ExplosionSound);

	// Shove nearby physics-simulating objects away from the blast.
	RadialForceComponent->FireImpulse();

}
	