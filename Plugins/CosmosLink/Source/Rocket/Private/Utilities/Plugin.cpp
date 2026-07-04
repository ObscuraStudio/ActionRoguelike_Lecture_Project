// Copyright 2024 Leartes Studios. All Rights Reserved.

#include "Plugin.h"
#include "Database.h"
#include "Rocket.h"
#include "SWebBrowser.h"
#include "SourceManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"

FString FPlugin::GetVersion() {
	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("Rocket"));

	if (Plugin.IsValid()) {
		const FPluginDescriptor &Descriptor = Plugin->GetDescriptor();
		return Descriptor.VersionName;
	}

	return FString("Unknown Version");
}

void FPlugin::MakeBrowserVisible() {
	FRocket::WebBrowserWidget->SetVisibility(EVisibility::Visible);
}

void FPlugin::ReloadWebBrowser() {
	FRocket::WebBrowserWidget->Reload();
}

void FPlugin::ResetToFactoryDefaults() {
	FDatabase::Shutdown();
	FSourceManager::DeleteRocketFolder();
	FSourceManager::InitializeSourceDirectory();
	FDatabase::Initialize();
}
