// Copyright Narrative Tools 2025.

#pragma once

#include "NarrativeNodeSelector.generated.h"

class UQuest;
class UDialogue;

USTRUCT(BlueprintType, meta=(HiddenByDefault, BlueprintInternalUseOnly="true"))
struct NARRATIVE_API FNodeIDSelector
{
	GENERATED_BODY()
	
	// actual node id
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NodeID")
	FName NodeID;
	
};

// selects dialogue nodes
USTRUCT(BlueprintType, meta=(HasNativeMake="/Script/Narrative.NarrativeFunctionLibrary.MakeDialogueNodeSelector", HasNativeBreak="/Script/Narrative.NarrativeFunctionLibrary.BreakDialogueNodeSelector"))
struct NARRATIVE_API FDialogueNodeSelector final : public FNodeIDSelector
{
	GENERATED_BODY()

	// dialogue asset 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NodeID")
	TSoftClassPtr<UDialogue> Asset;
	
};

// base type for quest node selectors
USTRUCT(NotBlueprintType, meta=(HiddenByDefault, BlueprintInternalUseOnly="true"))
struct FQuestNodeSelector : public FNodeIDSelector
{
	GENERATED_BODY()
	
	// quest asset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NodeID")
	TSoftClassPtr<UQuest> Asset;
	
};

// selects quest state nodes
USTRUCT(BlueprintType, meta=(HasNativeMake="/Script/Narrative.NarrativeFunctionLibrary.MakeQuestStateSelector", HasNativeBreak="/Script/Narrative.NarrativeFunctionLibrary.BreakQuestStateSelector"))
struct NARRATIVE_API FQuestStateSelector final : public FQuestNodeSelector
{
	GENERATED_BODY()
};

// selects quest branch nodes
USTRUCT(BlueprintType, meta=(HasNativeMake="/Script/Narrative.NarrativeFunctionLibrary.MakeQuestBranchSelector", HasNativeBreak="/Script/Narrative.NarrativeFunctionLibrary.BreakQuestBranchSelector"))
struct NARRATIVE_API FQuestBranchSelector final : public FQuestNodeSelector
{
	GENERATED_BODY()
};

