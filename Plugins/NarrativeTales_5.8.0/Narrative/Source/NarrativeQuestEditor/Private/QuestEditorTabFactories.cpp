// Copyright Narrative Tools 2025. 

#include "QuestEditorTabFactories.h"
#include "QuestBlueprint.h"
#include "Widgets/Docking/SDockTab.h"
#include "QuestEditorTabs.h"
#include "Engine/Blueprint.h"
#include "QuestEditorStyle.h"
 
#define LOCTEXT_NAMESPACE "QuestGraphEditorFactories"

FQuestGraphEditorSummoner::FQuestGraphEditorSummoner(TSharedPtr<class FQuestGraphEditor> InQuestEditorPtr)
	: FDocumentTabFactoryForObjects<UEdGraph>(FQuestEditorTabs::GraphEditorID, InQuestEditorPtr)
	, QuestEditorPtr(InQuestEditorPtr)
{

}

void FQuestGraphEditorSummoner::OnTabActivated(TSharedPtr<SDockTab> Tab) const
{
	check(QuestEditorPtr.IsValid());	
	TSharedRef<SGraphEditor> GraphEditor = StaticCastSharedRef<SGraphEditor>(Tab->GetContent());
	QuestEditorPtr.Pin()->OnGraphEditorFocused(GraphEditor);
}

void FQuestGraphEditorSummoner::OnTabRefreshed(TSharedPtr<SDockTab> Tab) const
{
	TSharedRef<SGraphEditor> GraphEditor = StaticCastSharedRef<SGraphEditor>(Tab->GetContent());
	GraphEditor->NotifyGraphChanged();
}

TAttribute<FText> FQuestGraphEditorSummoner::ConstructTabNameForObject(UEdGraph* DocumentID) const
{
	return TAttribute<FText>(FText::FromString(DocumentID->GetName()));
}

TSharedRef<SWidget> FQuestGraphEditorSummoner::CreateTabBodyForObject(const FWorkflowTabSpawnInfo& Info, UEdGraph* DocumentID) const
{

	check(QuestEditorPtr.IsValid());
	return QuestEditorPtr.Pin()->CreateQuestGraphEditorWidget(DocumentID);
}

const FSlateBrush* FQuestGraphEditorSummoner::GetTabIconForObject(const FWorkflowTabSpawnInfo& Info, UEdGraph* DocumentID) const
{
	return FQuestEditorStyle::Get()->GetBrush("ClassIcon.Quest");
}

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 6
using FGraphLocationType = FVector2f;
#else
using FGraphLocationType = FVector2D;
#endif

void FQuestGraphEditorSummoner::SaveState(TSharedPtr<SDockTab> Tab, TSharedPtr<FTabPayload> Payload) const
{
	check(QuestEditorPtr.IsValid());
	check(QuestEditorPtr.Pin()->GetQuestAsset());


	TSharedRef<SGraphEditor> GraphEditor = StaticCastSharedRef<SGraphEditor>(Tab->GetContent());

	FGraphLocationType ViewLocation;
	float ZoomAmount;
	GraphEditor->GetViewLocation(ViewLocation, ZoomAmount);

	UEdGraph* Graph = FTabPayload_UObject::CastChecked<UEdGraph>(Payload);
	QuestEditorPtr.Pin()->GetQuestAsset()->LastEditedDocuments.Add(FEditedDocumentInfo(Graph, ViewLocation, ZoomAmount));
}

FQuestDetailsSummoner::FQuestDetailsSummoner(TSharedPtr<class FQuestGraphEditor> InQuestEditorPtr)
	: FWorkflowTabFactory(FQuestEditorTabs::GraphDetailsID, InQuestEditorPtr)
	, QuestEditorPtr(InQuestEditorPtr)
{

	TabLabel = LOCTEXT("QuestDetailsLabel", "Details");
	TabIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "Kismet.Tabs.Components");

	bIsSingleton = true;


	ViewMenuDescription = LOCTEXT("QuestDetailsView", "Details");
	ViewMenuTooltip = LOCTEXT("QuestDetailsView_ToolTip", "Show the details view");

}

FText FQuestDetailsSummoner::GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const
{
	return LOCTEXT("QuestDetailsTabTooltip", "The quest editor details tab allows editing of quest graph nodes");
}

#undef LOCTEXT_NAMESPACE
