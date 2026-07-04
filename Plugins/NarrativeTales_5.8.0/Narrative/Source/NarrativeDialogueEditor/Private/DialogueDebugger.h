// Copyright Narrative Tools 2025.

#pragma once

#include "Tickable.h"
#include "NarrativeComponent.h"

class UDialogueGraphNode_Root;
class FDialogueGraphEditor;

class FDialogueDebugger final : public FTickableGameObject, public TSharedFromThis<FDialogueDebugger>
{
	friend FDialogueGraphEditor;
	
public:
	
	FDialogueDebugger();
	~FDialogueDebugger();
	
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual bool IsTickableWhenPaused() const override { return true; }
	virtual bool IsTickableInEditor() const override { return true; }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(FDialogueDebuggerTickHelper, STATGROUP_Tickables); }

	void Setup(UDialogueBlueprint* DialogueBlueprint, TSharedRef<FDialogueGraphEditor> OwnerEditor);

	void BindCommands(TSharedRef<FUICommandList> EditorCommandList);
	void RegisterDebuggerToolbar(UToolMenu* ToolMenu);
	
	static bool IsPIENotSimulating();

	bool IsDebuggerReady() const { return bIsPIEActive; }
	
private:
	void OnBeginPIE(const bool bIsSimulating);
	void OnEndPIE(const bool bIsSimulating);
	void OnDialogueStarted(UNarrativeComponent* TalesComponent, UDialogue* Dialogue, const FName& Name);
	void OnDialogueEnd(UNarrativeComponent* TalesComponent, UDialogue* Dialogue);
	
	void DebuggerJumpToNode();
	
	void SetNodeActive(const FName& NodeNodeID, const bool Active);
	void ResetActiveNode();
	
protected:

	uint8 bIsPIEActive : 1;
	
	TWeakObjectPtr<UNarrativeComponent> TalesComponentInstance;
	
	/// owning editor
	TWeakPtr<FDialogueGraphEditor> EditorOwner;

	/// asset for debugging
	TWeakObjectPtr<UDialogueBlueprint> DialogueBlueprintAsset;

	TWeakObjectPtr<UDialogue> ActiveDialogue;

	TWeakObjectPtr<UDialogueNode> ActiveNode;
	
};
