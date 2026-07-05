// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/RogueInteractionInterface.h"
#include "GameFramework/Actor.h"
#include "RogueItemChest.generated.h"

class UStaticMeshComponent;

UCLASS()
class ACTIONROGUELIKE_API ARogueItemChest : public AActor, public IRogueInteractionInterface
{
	GENERATED_BODY()

	
protected:

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BaseMeshComponent;
	
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> LidMeshComponent;

	float CurrentAnimationPitch = 0.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	float AnimationSpeed = 60.f;	
	
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	float AnimationTargetPitch = 120.0f;
	
	void Tick(float DeltaTime);

public:

	virtual void Interact() override;
	
	ARogueItemChest();
	
};

