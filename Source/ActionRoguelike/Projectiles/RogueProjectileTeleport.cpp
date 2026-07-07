// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueProjectileTeleport.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "TimerManager.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/ProjectileMovementComponent.h"

ARogueProjectileTeleport::ARogueProjectileTeleport()
{
	ProjectileMovementComponent->InitialSpeed = 6000.0f;
}

void ARogueProjectileTeleport::BeginPlay()
{
	Super::BeginPlay();
	
	GetWorldTimerManager().SetTimer(TeleportHandle, this, &ARogueProjectileTeleport::StartDelayedTeleport, DetonateDelay);
}

void ARogueProjectileTeleport::OnActorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	// On hit with the world: same behavior as the timed detonation.
	GetWorldTimerManager().ClearTimer(TeleportHandle);
	
	StartDelayedTeleport();
}

void ARogueProjectileTeleport::StartDelayedTeleport()
{
	// No cast needed: GetInstigator() is the player pawn that fired us.
	if (APawn* MyInstigator = GetInstigator())
	{
		
		PlayExplodeEffects();
		
		ProjectileMovementComponent->StopMovementImmediately();
		LoopedNiagaraComponent->Deactivate();
		LoopedAudioComponent->Stop();
		SetActorEnableCollision(false);
		GetWorldTimerManager().SetTimer(TeleportHandle, this, &ARogueProjectileTeleport::HandleTeleport, TeleportDelay);
		
		ProjectileMovementComponent->StopMovementImmediately();
		
	}
	
}

void ARogueProjectileTeleport::HandleTeleport()
{
	APawn* ActorToTeleport = GetInstigator();
	check(ActorToTeleport);
	
	ActorToTeleport->TeleportTo(GetActorLocation(), ActorToTeleport->GetActorRotation());
	
	Destroy();
}
	

