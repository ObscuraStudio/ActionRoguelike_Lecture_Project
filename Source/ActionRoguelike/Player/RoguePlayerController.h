// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RogueInteractionComponent.h"
#include "GameFramework/PlayerController.h"
#include "RoguePlayerController.generated.h"

class UInputAction;

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API ARoguePlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	TObjectPtr<URogueInteractionComponent> InteractionComponent;
	
	UPROPERTY(EditDefaultsOnly, Category = "Inputs")
	TObjectPtr<UInputAction> Input_Interact;
	
	void StartInteract();
	
	
	virtual void SetupInputComponent() override;
	
public:
	
	ARoguePlayerController();
	
};
