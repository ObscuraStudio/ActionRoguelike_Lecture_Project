// Copyright Narrative Tools 2025. 


#include "NarrativeEvent.h"

#include "NarrativeComponent.h"
#include "NarrativeCondition.h"
#include "NarrativePartyComponent.h"
#include "Chaos/PBDSuspensionConstraintData.h"


UNarrativeEvent::UNarrativeEvent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bRefireOnLoad = true; 
}

void UNarrativeEvent::OnActivate_Implementation(APawn* Target, APlayerController* Controller, class UNarrativeComponent* NarrativeComponent)
{
	ExecuteEvent(Target, Controller, NarrativeComponent);
}

void UNarrativeEvent::OnDeactivate_Implementation(APawn* Target, APlayerController* Controller, class UNarrativeComponent* NarrativeComponent)
{

}

void UNarrativeEvent::ExecuteEvent_Implementation(APawn* Pawn, APlayerController* Controller, class UNarrativeComponent* NarrativeComponent)
{
	
}

FString UNarrativeEvent::GetGraphDisplayText_Implementation()
{
	return GetName();
}

FText UNarrativeEvent::GetHintText_Implementation()
{
	return FText::GetEmpty();
}

bool UNarrativeEvent::AreConditionsMet(APawn* Pawn, APlayerController* Controller, class UNarrativeComponent* NarrativeComponent)
{

	if (!NarrativeComponent)
	{
		UE_LOG(LogNarrative, Warning, TEXT("Tried running conditions on node %s but Narrative Comp was null."), *GetNameSafe(this));
		return false;
	}
	  
	//Ensure all conditions are met
	for (auto& Cond : Conditions)
	{	
		if (Cond)
		{
			//We're running a condition on a party! Figure out who we need to run the condition on
			if (UNarrativePartyComponent* PartyComp = Cast<UNarrativePartyComponent>(NarrativeComponent))
			{
				TArray<UNarrativeComponent*> ComponentsToCheck;

				UE_LOG(LogNarrative, Warning, TEXT("Running on party..."));
				//We need to check everyone in the party
				if (Cond->PartyConditionPolicy == EPartyConditionPolicy::AllPlayersPass || Cond->PartyConditionPolicy == EPartyConditionPolicy::AnyPlayerPasses)
				{
					ComponentsToCheck.Append(PartyComp->GetPartyMembers());
				}//We need to check the party leader
				else if (Cond->PartyConditionPolicy == EPartyConditionPolicy::PartyLeaderPasses)
				{
					ComponentsToCheck.Add(PartyComp->GetPartyLeader());
				}
				else if (Cond->PartyConditionPolicy == EPartyConditionPolicy::PartyPasses)
				{
					ComponentsToCheck.Add(PartyComp);
				}

				bool bAnyonePassed = false;

				//If any of our comps to check fail, return false 
				for (auto& ComponentToCheck : ComponentsToCheck)
				{	
					const bool bConditionPassed = Cond && Cond->CheckCondition(ComponentToCheck->GetOwningPawn(), ComponentToCheck->GetOwningController(), ComponentToCheck) != Cond->bNot;
					FString CondString = bConditionPassed ? "passed" : "failed";

					if (bConditionPassed)
					{
						//We'll check the next condition since someone passed
						if (Cond->PartyConditionPolicy == EPartyConditionPolicy::AnyPlayerPasses)
						{
							bAnyonePassed = true;
							break;
						}
					}
					else
					{
						if (Cond->PartyConditionPolicy != EPartyConditionPolicy::AnyPlayerPasses)
						{
							return false;
						}
					}

					UE_LOG(LogNarrative, Warning, TEXT("Checking %s event condition, and they: %s"), *GetNameSafe(ComponentToCheck), *CondString);
				}

				//If we didn't break, no players passed 
				if (!bAnyonePassed && Cond->PartyConditionPolicy == EPartyConditionPolicy::AnyPlayerPasses)
				{
					return false;
				}

			}
			else
			{
				if (Cond && Cond->CheckCondition(Pawn, Controller, NarrativeComponent) == Cond->bNot)
				{
					return false;
				}
			}

		}
	}

	return true;
}
