// Copyright Narrative Tools 2025. 

#include "DialogueGraphNode_Root.h"
#include "DialogueEditorTypes.h"

void UDialogueGraphNode_Root::AllocateDefaultPins()
{
	CreatePin(EGPD_Output, UDialogueEditorTypes::PinCategory_SingleNode, TEXT(""));
}

bool UDialogueGraphNode_Root::CanUserDeleteNode() const
{
	return false;
}