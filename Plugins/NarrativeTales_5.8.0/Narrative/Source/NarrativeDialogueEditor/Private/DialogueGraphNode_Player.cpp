// Copyright Narrative Tools 2025. 

#include "DialogueGraphNode_Player.h"
#include "EdGraph/EdGraphSchema.h"
#include "DialogueEditorTypes.h"
#include "DialogueEditorSettings.h"

#define LOCTEXT_NAMESPACE "DialogueGraphNode_Player"

void UDialogueGraphNode_Player::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, UDialogueEditorTypes::PinCategory_SingleNode, TEXT(""));
	CreatePin(EGPD_Output, UDialogueEditorTypes::PinCategory_SingleNode, TEXT(""));
}

FLinearColor UDialogueGraphNode_Player::GetGraphNodeColor() const
{
	return GetDefault<UDialogueEditorSettings>()->PlayerNodeColor;
}

FText UDialogueGraphNode_Player::GetNodeTitleText() const
{
	return LOCTEXT("PlayerNodeTitleText", "Player");
}

#undef LOCTEXT_NAMESPACE
