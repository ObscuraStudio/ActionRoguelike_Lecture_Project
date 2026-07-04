// Copyright Narrative Tools 2025. 


#include "NarrativeGameplayTags.h"

#include "Containers/Array.h"
#include "Engine/EngineTypes.h"
#include "GameplayTagsManager.h"
#include "Logging/LogCategory.h"
#include "Logging/LogMacros.h"
#include "Trace/Detail/Channel.h"
#include "UObject/NameTypes.h"

FNarrativeGameplayTags FNarrativeGameplayTags::GameplayTags;

void FNarrativeGameplayTags::InitializeNativeTags()
{
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

	GameplayTags.AddAllTags(Manager);

	// Notify manager that we are done adding native tags.
	Manager.DoneAddingNativeTags();
}

void FNarrativeGameplayTags::AddAllTags(UGameplayTagsManager& Manager)
{
	AddTag(State_DialogueControlled, "Narrative.State.DialogueControlled", "Tag is added to character if they are in a dialogue.");

	AddTag(TaggedDialogue_Greet, "Narrative.TaggedDialogue.Greet", "Fires the NPCs greet dialogue. 'Hello', 'How are you doing friend?' etc ");
	AddTag(TaggedDialogue_Farewell, "Narrative.TaggedDialogue.Farewell", "Fires the NPCs farewell dialogue. 'Farewell', 'See you later!' etc");
}

void FNarrativeGameplayTags::AddTag(FGameplayTag& OutTag, const ANSICHAR* TagName, const ANSICHAR* TagComment)
{
	OutTag = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TagName), FString(TEXT("(Native) ")) + FString(TagComment));
}

FGameplayTag FNarrativeGameplayTags::FindTagByString(FString TagString, bool bMatchPartialString)
{
	const UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
	FGameplayTag Tag = Manager.RequestGameplayTag(FName(*TagString), false);

	if (!Tag.IsValid() && bMatchPartialString)
	{
		FGameplayTagContainer AllTags;
		Manager.RequestAllGameplayTags(AllTags, true);

		for (const FGameplayTag TestTag : AllTags)
		{
			if (TestTag.ToString().Contains(TagString))
			{
				UE_LOG(LogTemp, Display, TEXT("Could not find exact match for tag [%s] but found partial match on tag [%s]."), *TagString, *TestTag.ToString());
				Tag = TestTag;
				break;
			}
		}
	}

	return Tag;
}
