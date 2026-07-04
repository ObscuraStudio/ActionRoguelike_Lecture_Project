//  Copyright Narrative Tools 2022.


#include "DialogueNodeUserWidget.h"


void UDialogueNodeUserWidget::InitializeFromNode(class UDialogueNode* InNode, class UDialogue* InDialogue)
{
	if (InNode)
	{
		Node = InNode;
		Dialogue = InDialogue;

		OnNodeInitialized(InNode, InDialogue);
	}
}
