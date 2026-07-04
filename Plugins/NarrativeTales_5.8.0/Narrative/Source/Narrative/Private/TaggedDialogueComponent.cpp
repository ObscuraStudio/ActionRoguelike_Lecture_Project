// Copyright Narrative Tools 2025. 

#include "TaggedDialogueComponent.h"

#include "Dialogue.h"
#include "NarrativeComponent.h"
#include "NarrativeFunctionLibrary.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

// Sets default values for this component's properties
UTaggedDialogueComponent::UTaggedDialogueComponent()
{
	SetIsReplicatedByDefault(true);
}

UTaggedDialogueSet::UTaggedDialogueSet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool UTaggedDialogueComponent::PlayTaggedDialogue_Implementation(FGameplayTag Tag, AActor* DialogueInstigator)
{
	// exit if no tagged dialogue is set
	UTaggedDialogueSet* Set = TaggedDialogueSet.LoadSynchronous();
	if (!Set)
	{
		return false;
	}
	
	for (auto& TaggedDialogue : Set->TaggedDialogues)
	{
		if (TaggedDialogue.Tag == Tag)
		{
			float& LastTime = LastPlayTimes.FindOrAdd(TaggedDialogue.Tag);

			// If never played, initialize so TimeSince > Cooldown on first play
			if (LastTime == 0.f)
			{
				LastTime = -TaggedDialogue.Cooldown;
			}
			
			const float TimeSince = GetWorld()->TimeSince(LastTime);
			
			if (TimeSince > TaggedDialogue.Cooldown)
			{
				bool played = ExecutePlayTaggedDialogue(TaggedDialogue, DialogueInstigator);
				
				if (played) // only play last play time if triggered successfully
				{
					LastTime = GetWorld()->GetTimeSeconds();
				}
				return played;
			}
		}
	}
	
	return false;
}

bool UTaggedDialogueComponent::ExecutePlayTaggedDialogue_Implementation(FTaggedDialogue Dialogue, AActor* DialogueInstigator)
{
	if (const auto& NarrativeComponent = UNarrativeFunctionLibrary::GetNarrativeComponentFromTarget(DialogueInstigator))
	{		
		if (UClass* LoadedClass = Dialogue.Dialogue.LoadSynchronous())
		{
			if (NarrativeComponent->HasDialogueAvailable(LoadedClass, Dialogue.PlayParams))
			{
				return NarrativeComponent->BeginDialogue(LoadedClass, Dialogue.PlayParams);
			}
		}
	}
	
	return false;
}
