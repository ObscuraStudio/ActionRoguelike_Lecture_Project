// Copyright 2024 Leartes Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Docking/TabManager.h"

class SWebBrowser;

class FRocket : public TSharedFromThis<FRocket> {
public:
	// Initializes the browser.
	static void Initialize();

	// Shuts down the browser.
	static void Shutdown();

	// Singleton instance of the browser.
	static TSharedPtr<FRocket> Instance;

	// Dock for the browser.
	TSharedPtr<SDockTab> BrowserDock;

	// Widget for the web browser.
	static TSharedPtr<SWebBrowser> WebBrowserWidget;

private:
	// Registers the menus.
	static void RegisterMenus();
	static void OpenBrowserRequested();
	static void FillToolbar(FToolBarBuilder &ToolbarBuilder);

	// Creates the browser widget.
	static void CreateBrowserWidget();

	// Spawns a new plugin tab.
	TSharedRef<SDockTab> OnGenerateRocketTab(const FSpawnTabArgs &SpawnTabArgs);

	TSharedPtr<FUICommandList> PluginCommands; // List of commands for the plugin.
};
