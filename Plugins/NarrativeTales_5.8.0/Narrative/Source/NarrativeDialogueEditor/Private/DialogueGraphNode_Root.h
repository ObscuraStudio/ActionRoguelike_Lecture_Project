// Copyright Narrative Tools 2025. 

#pragma once

#include "CoreMinimal.h"
#include "DialogueGraphNode_NPC.h"
#include "DialogueGraphNode_Root.generated.h"

/**
 * 
 */
UCLASS()
class UDialogueGraphNode_Root : public UDialogueGraphNode_NPC
{
	GENERATED_BODY()

public:

	virtual void AllocateDefaultPins() override;
	virtual bool CanDuplicateNode() const override { return false; }
	virtual bool CanUserDeleteNode() const override;
	
};
