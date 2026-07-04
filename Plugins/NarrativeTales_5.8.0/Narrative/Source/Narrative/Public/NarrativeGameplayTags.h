// Copyright Narrative Tools 2025. 

#pragma once

#include "CoreMinimal.h"
#include "Containers/Map.h"
#include "Containers/UnrealString.h"
#include "GameplayTagContainer.h"
#include "HAL/Platform.h"

class UGameplayTagsManager;

/**
 * Gameplay tags singleton class, we use a technique learned from Lyra. 
 * We define these natively so Narrative pro users don't need to manually add a whole bunch of tags, which would be quite annoying.
 * This way they can hit the ground running. 
 */
struct NARRATIVE_API FNarrativeGameplayTags
{
public:

	static const FNarrativeGameplayTags& Get() { return GameplayTags; }

	static void InitializeNativeTags();

	static FGameplayTag FindTagByString(FString TagString, bool bMatchPartialString = false);

public:
	
	FGameplayTag State_DialogueControlled;

	FGameplayTag TaggedDialogue_Greet;
	FGameplayTag TaggedDialogue_Farewell;
	FGameplayTag TaggedDialogue_Taunt;
	FGameplayTag TaggedDialogue_Attack;
	FGameplayTag TaggedDialogue_BeginAttacking;

	FGameplayTag TaggedDialogue_Investigate_HeardSound_StartSearch;
	FGameplayTag TaggedDialogue_Investigate_HeardSound_CouldntFindAnything;
	FGameplayTag TaggedDialogue_Investigate_HeardSound_FoundEnemy;
	FGameplayTag TaggedDialogue_Investigate_SearchForEnemy_StartSearch;
	FGameplayTag TaggedDialogue_Investigate_SearchForEnemy_FoundEnemy;
	FGameplayTag TaggedDialogue_Investigate_SearchForEnemy_CouldntFindEnemy;

	FGameplayTag TaggedDialogue_DidntFindEnemy;
	FGameplayTag TaggedDialogue_FriendlyFire;

protected:

	void AddAllTags(UGameplayTagsManager& Manager);
	void AddTag(FGameplayTag& OutTag, const ANSICHAR* TagName, const ANSICHAR* TagComment);

private:

	static FNarrativeGameplayTags GameplayTags;
};
