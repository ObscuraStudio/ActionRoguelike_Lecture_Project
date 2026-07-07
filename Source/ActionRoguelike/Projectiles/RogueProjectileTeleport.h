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

	UPROPERTY(EditDefaultsOnly, Category="Explosion")
	float DetonateDelay = 0.5f;
	
	UPROPERTY(EditDefaultsOnly, Category="Explosion")
	float TeleportDelay = 0.5f;
	
	FTimerHandle TeleportHandle;
	
	void StartDelayedTeleport();
    	
	void HandleTeleport();
	
	virtual void BeginPlay() override;
	
	virtual void OnActorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
    		FVector NormalImpulse, const FHitResult& Hit) override;
	
public:
	
	ARogueProjectileTeleport();

};
