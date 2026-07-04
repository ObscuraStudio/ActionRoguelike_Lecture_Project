// Copyright 2024 Leartes Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NetworkDataContext.h"
#include "RequiredPluginsWindow.h"

struct FPluginViewItemInfo;

namespace PluginNames
{
	const FString EnhancedInput = TEXT("EnhancedInput");
	const FString ChaosVehicles = TEXT("ChaosVehiclesPlugin");
}

namespace ConfigFiles
{
	const FString DefaultInput = TEXT("DefaultInput.ini");
}


static const TMap<FString, TMap<FString, TMap<FString, TMap<FString, FString>>>> PluginConfigMap = {
	{ ConfigFiles::DefaultInput, {  
	        { PluginNames::EnhancedInput, {  
	            { TEXT("/Script/Engine.InputSettings"), {  
	                { TEXT("DefaultPlayerInputClass"), TEXT("/Script/EnhancedInput.EnhancedPlayerInput") },
					{ TEXT("DefaultInputComponentClass"), TEXT("/Script/EnhancedInput.EnhancedInputComponent") }
	            }}
	        }}
	}},
};


class FRocketDependencyManager {

public:
	//Asset
	static TArray<FString> FindPluginDependencies(const FString& InAssetType,const FString& InObjectType,bool bInIsInteractable);
	
	//Environment
	static TArray<FString> GetEnabledPlugins(const FString& ProjectFolderPath);
	static bool TryEnablePlugin(const FString &PluginName);
	static bool CheckIfEnginePlugin(const FString &PluginName);
	static bool IsPluginEnabled(const FString &PluginName);
	static TArray<FPluginViewItemInfo> CreatePluginViewItemInfoArray(const TArray<FString>& MustActivateEnginePluginList);
	static void SetupPluginConfigRequirements(const FString& PluginName);

	static void SuggestRestart();
	
private:
	// Function to return the dependencies for a specific object type
	static TArray<FString> GetPluginsDependencies(ERocketObjectType ObjectType, bool bIsInteractable);
	
	// Static map for storing dependencies
	static const TMap<ERocketObjectType, TArray<FString>>& InitializePluginsDependencies()
	{
		// The map is initialized only once when first accessed
		static TMap<ERocketObjectType, TArray<FString>> DependenciesMap = CreatePluginDependenciesMap();
		return DependenciesMap;
	}

	// Helper function to create and populate the map
	static TMap<ERocketObjectType, TArray<FString>> CreatePluginDependenciesMap()
	{
		TMap<ERocketObjectType, TArray<FString>> DependenciesMap;
		DependenciesMap.Add(ERocketObjectType::Vehicle, { PluginNames::EnhancedInput,PluginNames::ChaosVehicles});
		return DependenciesMap;
	}
};


class FRocketDependencyUIManager {
public:
	static TArray<FName> SpawnPluginEnableWindow(const FString& InProductID, const FText& InMessage, const TArray<FPluginViewItemInfo>& PluginViewInfo, const FOnPluginsActivationResponseSignature& OnPluginsActivationResponse);
	static void ResetWindowData();
private:
	static TSharedPtr<class SWindow> PluginActivationWindow;
};