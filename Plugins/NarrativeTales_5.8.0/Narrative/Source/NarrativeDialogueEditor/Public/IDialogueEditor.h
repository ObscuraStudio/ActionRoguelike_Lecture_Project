// Copyright Narrative Tools 2025. 

#pragma once

#include "CoreMinimal.h"
#include "BlueprintEditor.h"

/** Quest editor public interface */
class IDialogueEditor : public FBlueprintEditor
{
public: 

	// called when the dialogue icon is clicked on
	virtual void JumpOrAddDialogueNode(FName NodeID) = 0;
	
};
