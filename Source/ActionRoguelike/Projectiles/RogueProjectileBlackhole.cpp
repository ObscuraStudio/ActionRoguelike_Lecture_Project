// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueProjectileBlackhole.h"

#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/ProjectileMovementComponent.h"


// Sets default values
ARogueProjectileBlackhole::ARogueProjectileBlackhole()
{
	RadialForceComponent = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
	RadialForceComponent->SetupAttachment(RootComponent);
	// ForceStrength is applied continuously every tick (URadialForceComponent::TickComponent);
	// negative pulls objects inward. ImpulseStrength is only used by FireImpulse(), which the
	// blackhole never calls (see ExplosiveBarrel for the one-shot impulse use case).
	RadialForceComponent->ForceStrength = -800000.0f;
	RadialForceComponent->Radius = 1200.0f;
	RadialForceComponent->RemoveObjectTypeToAffect(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	
	SphereComponent->SetSphereRadius(20.0f);
	SphereComponent->SetCollisionProfileName("BlackholeCore");
	
	ProjectileMovementComponent->InitialSpeed = 500.0f;
	
	InitialLifeSpan = 5.0f;
}

void ARogueProjectileBlackhole::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ARogueProjectileBlackhole::OnSphereOverlappedActor);
}

void ARogueProjectileBlackhole::OnSphereOverlappedActor(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherComp->IsSimulatingPhysics())
	{
		OtherActor->Destroy();
	}
}

