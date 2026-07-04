// Copyright Narrative Tools 2025. 

#include "QuestGraphNode_PersistentTasks.h"
#include "QuestEditorTypes.h"
#include "QuestEditorSettings.h"

void UQuestGraphNode_PersistentTasks::AllocateDefaultPins()
{
	CreatePin(EGPD_Output, UQuestEditorTypes::PinCategory_SingleNode, TEXT(""));
}

bool UQuestGraphNode_PersistentTasks::CanUserDeleteNode() const
{
	return false;
}

FLinearColor UQuestGraphNode_PersistentTasks::GetGraphNodeColor() const
{
	return GetDefault<UQuestEditorSettings>()->PersistentTasksNodeColor;
}
