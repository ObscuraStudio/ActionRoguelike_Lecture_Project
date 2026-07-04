// Copyright 2024 Leartes Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FCommands {
public:
	static void RegisterCommands();
	static void UnregisterCommands();

private:
	static IConsoleCommand *ReloadBrowserCommand;
	static IConsoleCommand *ResetToFactoryDefaultsCommand;
	static IConsoleCommand *VersionCommand;
	static void HandleReloadBrowserCommand(const TArray<FString> &Args);
	static void HandleResetToFactoryDefaultsCommand(const TArray<FString> &Args);
	static void HandleVersionCommand(const TArray<FString> &Args);
};
