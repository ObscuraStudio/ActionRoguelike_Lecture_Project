
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TimerHandle.h"
#include "ExplosiveBarrel.generated.h"

class UAudioComponent;
class URadialForceComponent;
class UStaticMeshComponent;
class UNiagaraComponent;
class UDamageType;
class UNiagaraSystem;
class USoundBase;

UCLASS()
class ACTIONROGUELIKE_API AExplosiveBarrel : public AActor
{
	GENERATED_BODY()

protected:
	
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	TSubclassOf<UDamageType> DmgTypeClass;
	
	UPROPERTY(EditDefaultsOnly, Category="Effects")
	TObjectPtr<UNiagaraSystem> ExplosionEffect;
	
	UPROPERTY(EditDefaultsOnly, Category="Effects")
	TObjectPtr<UNiagaraSystem> IgnitionEffect;
	
	UPROPERTY(EditDefaultsOnly, Category="Sound")
	TObjectPtr<USoundBase> ExplosionSound;
	
	UPROPERTY(EditDefaultsOnly, Category="Sound")
	TObjectPtr<USoundBase> IgnitionSound;

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
	
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	TObjectPtr<URadialForceComponent> RadialForceComponent;
	
	UPROPERTY(EditDefaultsOnly, Category="Components")
	TObjectPtr<UNiagaraComponent> LoopedNiagaraComponent;
	
	UPROPERTY(EditDefaultsOnly, Category="Components")
	TObjectPtr<UAudioComponent> LoopedAudioComponent;
	
	UFUNCTION()
	void OnActorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit );
	
	UFUNCTION(BlueprintCallable, Category = "Explosion")
	void Explode();

	// Handle for the single explosion timer started on ignition.
	FTimerHandle ExplosionTimerHandle;

	// Guards against re-igniting when the barrel is hit multiple times.
	bool bIgnited = false;
	
	UPROPERTY(EditDefaultsOnly, Category="Explosion")
	float ExplosionDelayTime = 3.0f;

public:
	
	virtual void PostInitializeComponents() override;
	
	AExplosiveBarrel();
};
