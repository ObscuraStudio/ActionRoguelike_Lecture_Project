// Copyright 2024 Leartes Studios. All Rights Reserved.


#include "DependencyManager.h"
#include "AssetUtilities.h"
#include "GameProjectGenerationModule.h"
#include "RequiredPluginsWindow.h"
#include "Interfaces/IPluginManager.h"
#include "Interfaces/IProjectManager.h"
#include "Misc/Paths.h"
#include "Misc/MessageDialog.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "ISettingsEditorModule.h"
#include "RocketModule.h"

TSharedPtr<SWindow> FRocketDependencyUIManager::PluginActivationWindow = nullptr;

TArray<FString> FRocketDependencyManager::FindPluginDependencies(const FString& InAssetType,const FString& InObjectType,bool bInIsInteractable) {
	
	if(InObjectType.IsEmpty()){return {};}
	ERocketObjectType ObjectType = FAssetUtilities::GetObjectTypeFromString(InObjectType);
	return GetPluginsDependencies(ObjectType,bInIsInteractable);
}

TArray<FString> FRocketDependencyManager::GetEnabledPlugins(const FString& ProjectFolderPath)
{
	TArray<FString> EnabledPlugins;
    
	FString ProjectFilePath = ProjectFolderPath / FPaths::GetCleanFilename(ProjectFolderPath) + TEXT(".uproject");

	FString FileContents;
	if (!FFileHelper::LoadFileToString(FileContents, *ProjectFilePath))
	{
		UE_LOG(LogRocket, Error, TEXT("Unable to load .uproject file: %s"), *ProjectFilePath);
		return EnabledPlugins; 
	}

	TSharedPtr<FJsonObject> ProjectJson;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(FileContents);

	if (!FJsonSerializer::Deserialize(Reader, ProjectJson) || !ProjectJson.IsValid())
	{
		UE_LOG(LogRocket, Error, TEXT(".uproject file not in Json format or corrupted: %s"), *ProjectFilePath);
		return EnabledPlugins;
	}

	const TArray<TSharedPtr<FJsonValue>>* PluginsArray;
	if (ProjectJson->TryGetArrayField(TEXT("Plugins"), PluginsArray))
	{
		for (const TSharedPtr<FJsonValue>& PluginValue : *PluginsArray)
		{
			TSharedPtr<FJsonObject> PluginObject = PluginValue->AsObject();
			if (PluginObject.IsValid())
			{
				if (PluginObject->GetBoolField(TEXT("Enabled")))
				{
					FString PluginName = PluginObject->GetStringField(TEXT("Name"));
					EnabledPlugins.Add(PluginName);
				}
			}
		}
	}

	return EnabledPlugins;
}

bool FRocketDependencyManager::TryEnablePlugin(const FString& PluginName)
{
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(PluginName);
	if (!Plugin.IsValid())
	{
		UE_LOG(LogRocket, Error, TEXT("Plugin '%s' not found."), *PluginName);
		return false;
	}

	if (Plugin->IsEnabled())
	{
		UE_LOG(LogRocket, Log, TEXT("Plugin '%s' already enabled."), *PluginName);
		return true;
	}

	FText FailMessage;
	bool bSuccess = IProjectManager::Get().SetPluginEnabled(PluginName, true, FailMessage);

	if (bSuccess && IProjectManager::Get().IsCurrentProjectDirty())
	{
		FGameProjectGenerationModule::Get().TryMakeProjectFileWriteable(FPaths::GetProjectFilePath());
		bSuccess = IProjectManager::Get().SaveCurrentProjectToDisk(FailMessage);
	}

	if (bSuccess)
	{
		SetupPluginConfigRequirements(PluginName);
	}
	
	return bSuccess;
}

void FRocketDependencyManager::SetupPluginConfigRequirements(const FString& PluginName)
{
	bool bFoundConfig = false;

	// Loop through each config file level in PluginConfigMap
	for (const auto& ConfigFilePair : PluginConfigMap)
	{
		const FString& ConfigFileName = ConfigFilePair.Key;
		const TMap<FString, TMap<FString, TMap<FString, FString>>>& PluginMap = ConfigFilePair.Value;

		// Look up the specific plugin within the config file's plugin map
		if (const TMap<FString, TMap<FString, FString>>* PluginSettings = PluginMap.Find(PluginName))
		{
			bFoundConfig = true;
			const FString ConfigFilePath = FPaths::Combine(FPaths::ProjectConfigDir(), ConfigFileName);

			// Loop through each section within the plugin's settings
			for (const auto& SectionPair : *PluginSettings)
			{
				const FString& Section = SectionPair.Key;
				const TMap<FString, FString>& Settings = SectionPair.Value;

				// Loop through each key-value pair within the section
				for (const auto& KeyValuePair : Settings)
				{
					const FString& Key = KeyValuePair.Key;
					const FString& Value = KeyValuePair.Value;
					GConfig->SetString(*Section, *Key, *Value, ConfigFilePath);
					//UE_LOG(LogRocket, Log, TEXT("Applied setting [%s] %s = %s in %s"), *Section, *Key, *Value, *ConfigFilePath);
				}
			}
			
			// Flush changes to the specific config file
			GConfig->Flush(false, ConfigFilePath);
		}
	}

	if (!bFoundConfig)
	{
		UE_LOG(LogTemp, Warning, TEXT("No configuration found for plugin: %s"), *PluginName);
	}
}

bool FRocketDependencyManager::CheckIfEnginePlugin(const FString& PluginName)
{
	IPluginManager& PluginManager = IPluginManager::Get();
	TSharedPtr<IPlugin> Plugin = PluginManager.FindPlugin(PluginName);

	if (Plugin.IsValid())
	{
		FString PluginBaseDir = Plugin->GetBaseDir();
		FString EngineDir = FPaths::EngineDir();

		if (PluginBaseDir.StartsWith(EngineDir))
		{
			return true;
		}
	}
	return false;
}

bool FRocketDependencyManager::IsPluginEnabled(const FString& PluginName)
{
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(PluginName);
	if (!Plugin.IsValid())
	{
		return false;
	}
	
	return Plugin->IsEnabled();
}


TArray<FName> FRocketDependencyUIManager::SpawnPluginEnableWindow(const FString& InProductID, const FText& InMessage, const TArray<FPluginViewItemInfo>& InPluginViewInfo, const FOnPluginsActivationResponseSignature& OnPluginsActivationResponse)
{
	FSlateApplication& SlateApp = FSlateApplication::Get();
	const FVector2D WindowSize = FVector2D(345.f, 300.f);

	const TSharedRef<SRequiredPluginsWindow> ImportWindow = SNew(SRequiredPluginsWindow)
		.Title(FText::FromName(TEXT("Plugin Activation Required")))
		.Message(InMessage)
		.ProductID(*InProductID)
		.Plugins(InPluginViewInfo)
		.OnPluginsActivationResponse(OnPluginsActivationResponse);

	ImportWindow->MoveWindowTo(SlateApp.GetCursorPos() + FVector2D(-WindowSize.X / 2, -WindowSize.Y / 2));

	PluginActivationWindow = SlateApp.AddWindow(ImportWindow);
	if (PluginActivationWindow.IsValid())
	{
		PluginActivationWindow->Resize(FVector2D(WindowSize));
	}
	return {};
}

void SuggestRestart()
{
	if (ISettingsEditorModule* SettingsEditorModule = FModuleManager::GetModulePtr<ISettingsEditorModule>("SettingsEditor"))
	{
		SettingsEditorModule->OnApplicationRestartRequired();
	}
}

TArray<FPluginViewItemInfo> FRocketDependencyManager::CreatePluginViewItemInfoArray(const TArray<FString>& MustActivateEnginePluginList)
{
	TArray<FPluginViewItemInfo> PluginViewItemInfoArray;
	IPluginManager& PluginManager = IPluginManager::Get();

	for (const FString& PluginName : MustActivateEnginePluginList)
	{
		TSharedPtr<IPlugin> Plugin = PluginManager.FindPlugin(PluginName);
		if (Plugin.IsValid())
		{
			FString PluginDescription = Plugin->GetDescriptor().Description;
            
			FName PluginNameFName(*PluginName);
			FName PluginDescriptionFName(*PluginDescription);

			PluginViewItemInfoArray.Add(FPluginViewItemInfo(PluginNameFName, PluginDescriptionFName));
		}
	}

	return PluginViewItemInfoArray;
}

TArray<FString> FRocketDependencyManager::GetPluginsDependencies(ERocketObjectType ObjectType, bool bIsInteractable) 
{
	// Retrieve the full map of dependencies
	const TMap<ERocketObjectType, TArray<FString>>& DependenciesMap = InitializePluginsDependencies();

	auto IsVehicleAndInteractable = [ObjectType, bIsInteractable]() -> bool {
		return ObjectType == ERocketObjectType::Vehicle && bIsInteractable;
	};

	// Determine dependencies based on conditions
	if (IsVehicleAndInteractable()) {
		// If the object is a vehicle and interactable, return the dependencies for vehicles
		return DependenciesMap.FindChecked(ERocketObjectType::Vehicle);
	}
	else if (DependenciesMap.Contains(ObjectType)) {
		// If a valid dependency exists for the given object type, return it
		return DependenciesMap[ObjectType];
	}

	// Return an empty array if no conditions match
	return {};
}


void FRocketDependencyUIManager::ResetWindowData() {
	if (PluginActivationWindow.IsValid())
	{
		PluginActivationWindow->RequestDestroyWindow();
		PluginActivationWindow.Reset();
	}
}

void FRocketDependencyManager::SuggestRestart()
{
	if (ISettingsEditorModule* SettingsEditorModule = FModuleManager::GetModulePtr<ISettingsEditorModule>("SettingsEditor"))
	{
		SettingsEditorModule->OnApplicationRestartRequired();
	}
}
