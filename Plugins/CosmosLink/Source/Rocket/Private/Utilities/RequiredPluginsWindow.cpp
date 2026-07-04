// Copyright 2024 Leartes Studios. All Rights Reserved.


#include "RequiredPluginsWindow.h"
#include "SPrimaryButton.h"
#include "SWarningOrErrorBox.h"


void SPluginViewItemRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, const TSharedPtr<FPluginViewItemModel>& InModel)
{
	Model = InModel;
	ensure(Model->PluginViewItemInfoPtr.IsValid());
	
	SMultiColumnTableRow<FPluginViewItemInfoPtr>::Construct(FSuperRowType::FArguments(), InOwnerTableView);
}

TSharedRef<SWidget> SPluginViewItemRow::GenerateWidgetForColumn(const FName &ColumnName) {

	TSharedPtr<SWidget> TableRowContent = SNullWidget::NullWidget;
	
	if(ColumnName == TEXT("Plugin"))
	{
		TableRowContent =
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.AutoWidth()
		.Padding(FMargin(11,0,0,0))
		[
			SNew(SBox)
			.VAlign(VAlign_Center)
			.HeightOverride(20)
			[
				SNew(STextBlock)
				.Text(FText::FromName(Model->PluginViewItemInfoPtr->PluginName))
				.ToolTipText(FText::FromName(Model->PluginViewItemInfoPtr->PluginName))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
			]
		];
	}
	else if(ColumnName == TEXT("Description"))
	{
		TableRowContent =
		SNew(SHorizontalBox)
        			
		+SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(FMargin(11,0,0,0))
		[
			SNew(SBox)
			.VAlign(VAlign_Center)
			.HeightOverride(20)
			[
				SNew(STextBlock)
				.Text(FText::FromName(Model->PluginViewItemInfoPtr->Description))
				.ToolTipText(FText::FromName(Model->PluginViewItemInfoPtr->Description))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 8))
			]
		];
	}
	return TableRowContent.ToSharedRef();
}

void SRequiredPluginsWindow::Construct(const FArguments &InArgs) {

	Plugins = InArgs._Plugins;
	ProductID = InArgs._ProductID;
	OnPluginsActivationResponse = InArgs._OnPluginsActivationResponse;
	CreatePluginsViewItemModels();
	
	SWindow::Construct(SWindow::FArguments()
	.Title(InArgs._Title)
	.HasCloseButton(false)
	.IsTopmostWindow(true)
	.SupportsMaximize(false)
	.SupportsMinimize(false)
	.IsPopupWindow(false)
	.AutoCenter(EAutoCenter::PreferredWorkArea)
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot() // Add user input block
		.Padding(2,2,2,4)
		[
			SNew(SBorder)
			
			#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			#else
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			#endif
			
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.FillHeight(1)
				.Padding(2)
				[
					SAssignNew(TreeViewWidget,STreeView<FPluginViewItemModelPtr>)
					.TreeItemsSource(&PresetPipelineViewItemModels)
					.SelectionMode(ESelectionMode::Single)
					.ClearSelectionOnClick(true)
					.EnableAnimatedScrolling(true)
					.OnGenerateRow(this,&SRequiredPluginsWindow::TreeViewGenerateRow)
					.OnGetChildren_Lambda([](FPluginViewItemModelPtr Item, TArray<FPluginViewItemModelPtr>& OutChildren)
					{
						//No Children
					})
					.HeaderRow
					(
						// Toggle Active 
						SNew(SHeaderRow)
						+ SHeaderRow::Column(PluginViewTreeColumns::ColumnID_Plugin)
						.HeaderContentPadding(FMargin(2,2, 2, 2))
						.DefaultLabel(FText::FromName(PluginViewTreeColumns::ColumnID_Plugin))
						.HAlignHeader(HAlign_Center)
						.HAlignCell(HAlign_Left)
						.VAlignCell(VAlign_Center)
						.FillWidth(1.f)

						+ SHeaderRow::Column(PluginViewTreeColumns::ColumnID_Description)
						.HeaderContentPadding(FMargin(2,2, 2, 2))
						.DefaultLabel(FText::FromName(PluginViewTreeColumns::ColumnID_Description))
						.HAlignHeader(HAlign_Center)
						.HAlignCell(HAlign_Left)
						.VAlignCell(VAlign_Center)
						.FillWidth(3.0f)
					)
				]
				
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(3)
				[
					SNew(SWarningOrErrorBox)
					//.IconSize(FVector2D(16,16))
					.Padding(FMargin(1.0f))
					.MessageStyle(EMessageStyle::Warning)
					.Message(InArgs._Message)
				]
			]
		]

		+SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Bottom)
		.Padding(3.f, 16.f)
		[
			SNew(SHorizontalBox)
			
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.Padding(FMargin(10.0f,0.0f))
			[
				SAssignNew(ActionBtn,SPrimaryButton)
				.Text_Lambda([&]()
				{
					return FText::FromName(TEXT("Enable Plugins"));
				})
				.OnClicked(this, &SRequiredPluginsWindow::OnButtonClick, EAppReturnType::Ok)
			]
			
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.Padding(FMargin(10.0f,0.0f))
			[
				SNew(SButton)
				.Text(FText::FromName(TEXT("Cancel")))
				#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1)
				.ContentPadding(FAppStyle::GetMargin("StandardDialog.ContentPadding"))
				#else
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
				#endif
				.OnClicked(this, &SRequiredPluginsWindow::OnButtonClick, EAppReturnType::Cancel)
			]
		]
	]);
}

void SRequiredPluginsWindow::CreatePluginsViewItemModels() {

	if(Plugins.IsEmpty()){return;}
	
	for(const FPluginViewItemInfo& CurrentPlugin : Plugins)
	{
		FPluginViewItemModelPtr NewItemPtr = MakeShareable(new FPluginViewItemModel(MakeShareable(new FPluginViewItemInfo(CurrentPlugin.PluginName,CurrentPlugin.Description))));
			
		PresetPipelineViewItemModels.Emplace(NewItemPtr);
	}
}

TSharedRef<ITableRow> SRequiredPluginsWindow::TreeViewGenerateRow(FPluginViewItemModelPtr Item, const TSharedRef<STableViewBase> &OwnerTable) {

	return SNew(SPluginViewItemRow, OwnerTable, Item);
}

FReply SRequiredPluginsWindow::OnButtonClick(EAppReturnType::Type ButtonID) {

	if(OnPluginsActivationResponse.IsBound()) {

		OnPluginsActivationResponse.Execute(Plugins, ProductID, ButtonID == EAppReturnType::Ok);
	}

	return FReply::Handled();
}
