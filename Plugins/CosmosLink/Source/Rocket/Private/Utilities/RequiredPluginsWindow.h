// Copyright 2024 Leartes Studios. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"

// FOnPresetImportRequested: Triggered when a preset import is requested, passing the list of preset names
DECLARE_DELEGATE_ThreeParams(FOnPluginsActivationResponseSignature,const TArray<struct FPluginViewItemInfo>& PluginsInfo,const FName& OutProductID, bool OutResponse);

struct FPluginViewItemInfo
{
	FName PluginName;
	FName Description;
	
	FPluginViewItemInfo()
		: PluginName(FName()), Description(FName())
	{
	}

	// Full parameter constructor
	FPluginViewItemInfo(FName InPluginName, FName InDescription)
		: PluginName(InPluginName), Description(InDescription)
	{
	}
};

/** Type definition for shared pointers to instances of FEventGraphSample. */
typedef TSharedPtr<FPluginViewItemInfo> FPluginViewItemInfoPtr;

class FPluginViewItemModel : public TSharedFromThis<FPluginViewItemModel>
{
public:
	FPluginViewItemInfoPtr PluginViewItemInfoPtr;
	
	// Parameterized constructor
	FPluginViewItemModel(const FPluginViewItemInfoPtr& InPluginViewItemInfoPtr)
		: PluginViewItemInfoPtr(InPluginViewItemInfoPtr)
	{
	}
};

typedef TSharedPtr<FPluginViewItemModel> FPluginViewItemModelPtr;

namespace PluginViewTreeColumns
{
	/** IDs for list columns */
	static const FName ColumnID_Plugin("Plugin");
	static const FName ColumnID_Description("Description");
}

/** A tree row representing a foliage type in the palette */
class SPluginViewItemRow : public SMultiColumnTableRow<FPluginViewItemInfoPtr>
{
public:
	SLATE_BEGIN_ARGS(SPluginViewItemRow) {}
		
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, const TSharedPtr<FPluginViewItemModel>& InModel);
	
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

private:
	TSharedPtr<FPluginViewItemModel> Model;
};

// Class to represent a preset save window
class SRequiredPluginsWindow : public SWindow
{
public:
    SLATE_BEGIN_ARGS(SRequiredPluginsWindow){}

    SLATE_ARGUMENT(FText, Title)

    SLATE_ARGUMENT(FText, Message)

    SLATE_ARGUMENT(FName, ProductID)
    	
    SLATE_ARGUMENT(TArray<FPluginViewItemInfo>, Plugins)

    SLATE_EVENT(FOnPluginsActivationResponseSignature,OnPluginsActivationResponse)
	    
    SLATE_END_ARGS()

    // Default constructor
    SRequiredPluginsWindow(){}

    // Constructs a new SCMPresetPipelineExportWindow
    void Construct(const FArguments& InArgs);

private:
	FName ProductID;

	void CreatePluginsViewItemModels();

	TSharedRef<ITableRow> TreeViewGenerateRow(FPluginViewItemModelPtr Item,const TSharedRef<STableViewBase>& OwnerTable);
	
    // Handles button click events
    FReply OnButtonClick(EAppReturnType::Type ButtonID);
	
	FOnPluginsActivationResponseSignature OnPluginsActivationResponse;
	
    TArray<FPluginViewItemModelPtr> PresetPipelineViewItemModels;

    TSharedPtr<STreeView<FPluginViewItemModelPtr>> TreeViewWidget;
	
    TSharedPtr<class SPrimaryButton> ActionBtn;
	
	TArray<FPluginViewItemInfo> Plugins;
};
