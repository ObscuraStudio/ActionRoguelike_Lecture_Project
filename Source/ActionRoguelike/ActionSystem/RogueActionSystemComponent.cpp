// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueActionSystemComponent.h"




URogueActionSystemComponent::URogueActionSystemComponent()
{
	
}

void URogueActionSystemComponent::ApplyHealthChange(float InvalueChange)
{
	float OldHealth = Attributes.Health;
	
	Attributes.Health += InvalueChange;
	
	OnHealthChanged.Broadcast(Attributes.Health, OldHealth);
	
	UE_LOG(LogTemp, Log, TEXT("New Health: %f"), Attributes.Health);
}