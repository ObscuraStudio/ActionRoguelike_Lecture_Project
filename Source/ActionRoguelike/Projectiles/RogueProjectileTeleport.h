// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RogueProjectile.h"
#include "Engine/TimerHandle.h"
#include "RogueProjectileTeleport.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class UAudioComponent;

UCLASS(Abstract)
class ACTIONROGUELIKE_API ARogueProjectileTeleport : public ARogueProjectile
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditDefaultsOnly, Category="Effects")
	TObjectPtr<UNiagaraSystem> PortalEffect;

	UPROPERTY(EditDefaultsOnly, Category="Sound")
	TObjectPtr<USoundBase> PortalSound;
	
	UPROPERTY(EditDefaultsOnly, Category="Components")
	TObjectPtr<UAudioComponent> LoopedAudioComponent;
	
	UPROPERTY(EditDefaultsOnly, Category="Components")
	TObjectPtr<UNiagaraComponent> LoopedNiagaraComponent;

	UFUNCTION()
	void OnActorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);

	// Detonates: stops movement, plays the portal effect, then schedules the teleport.
	void Explode();

	// Teleports the instigator (the player that fired us) to the detonation point.
	void TeleportInstigator();
	
	void MethodWithDelay();

	FTimerHandle DetonationTimerHandle;
	FTimerHandle TeleportTimerHandle;

	// Ensures we only detonate once (a world hit and the timer could both fire).
	bool bExploded = false;

public:

	virtual void PostInitializeComponents() override;

ARogueProjectileTeleport();

};
