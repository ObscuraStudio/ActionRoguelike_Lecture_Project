// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueProjectileTeleport.h"

#include "AudioMixerTrace.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "TimerManager.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"


ARogueProjectileTeleport::ARogueProjectileTeleport()
{
	LoopedNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LoopedNiagaraComp"));
	LoopedNiagaraComponent->SetupAttachment(SphereComponent);
	LoopedAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("LoopedAudioComponent"));
	LoopedAudioComponent->SetupAttachment(SphereComponent);
	
	
}

void ARogueProjectileTeleport::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	SphereComponent->IgnoreActorWhenMoving(GetInstigator(), true);
	SphereComponent->OnComponentHit.AddDynamic(this, &ARogueProjectileTeleport::OnActorHit);
	
	GetWorldTimerManager().SetTimer(TeleportTimerHandle, this, &ARogueProjectileTeleport::TeleportInstigator, 2.0f);
}

void ARogueProjectileTeleport::OnActorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	// On hit with the world: same behavior as the timed detonation.
	Explode();
}

void ARogueProjectileTeleport::Explode()
{
	// Only detonate once — a world hit and the detonation timer could both call this.
	if (bExploded)
	{
		return;
	}
	bExploded = true;
	GetWorldTimerManager().ClearTimer(DetonationTimerHandle);

	// Stop the projectile in place while the detonation effect plays.
	ProjectileMovementComponent->StopMovementImmediately();

	// Play the detonation particle (and sound) at our current location.
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, PortalEffect, GetActorLocation(), GetActorRotation());
	if (PortalSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PortalSound, GetActorLocation());
	}

	// Let the effect play for a moment, then teleport the instigator.
	GetWorldTimerManager().SetTimer(TeleportTimerHandle, this, &ARogueProjectileTeleport::TeleportInstigator, 1.0f);
}

void ARogueProjectileTeleport::TeleportInstigator()
{
	// No cast needed: GetInstigator() is the player pawn that fired us.
	if (APawn* MyInstigator = GetInstigator())
	{
		ProjectileMovementComponent->StopMovementImmediately();
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, PortalEffect, GetActorLocation(), GetActorRotation());
		UGameplayStatics::PlaySoundAtLocation(this, PortalSound, GetActorLocation());
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &ARogueProjectileTeleport::MethodWithDelay, 0.5f);
	}
	
}

void ARogueProjectileTeleport::MethodWithDelay()
{
	GetInstigator()->TeleportTo(GetActorLocation(), GetInstigator()->GetActorRotation());
	
	Destroy();
}
	

