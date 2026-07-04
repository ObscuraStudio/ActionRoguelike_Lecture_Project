// Copyright 2024 Leartes Studios. All Rights Reserved.

#include "Commands.h"
#include "Plugin.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"

IConsoleCommand *FCommands::ReloadBrowserCommand = nullptr;
IConsoleCommand *FCommands::ResetToFactoryDefaultsCommand = nullptr;
IConsoleCommand *FCommands::VersionCommand = nullptr;

void FCommands::RegisterCommands() {
	ReloadBrowserCommand = IConsoleManager::Get().RegisterConsoleCommand(TEXT("Rocket.Reload"), TEXT("Hard reload custom page"), FConsoleCommandWithArgsDelegate::CreateStatic(HandleReloadBrowserCommand), ECVF_Default);
	ResetToFactoryDefaultsCommand = IConsoleManager::Get().RegisterConsoleCommand(TEXT("Rocket.ResetToFactoryDefaults"), TEXT("Reset to factory defaults"), FConsoleCommandWithArgsDelegate::CreateStatic(HandleResetToFactoryDefaultsCommand), ECVF_Default);
	VersionCommand = IConsoleManager::Get().RegisterConsoleCommand(TEXT("Rocket.Version"), TEXT("Get the plugin version"), FConsoleCommandWithArgsDelegate::CreateStatic(HandleVersionCommand), ECVF_Default);
}

void FCommands::UnregisterCommands() {
	if (ReloadBrowserCommand) {
		IConsoleManager::Get().UnregisterConsoleObject(ReloadBrowserCommand);
		ReloadBrowserCommand = nullptr;
	}
	if (ResetToFactoryDefaultsCommand) {
		IConsoleManager::Get().UnregisterConsoleObject(ResetToFactoryDefaultsCommand);
		ResetToFactoryDefaultsCommand = nullptr;
	}
	if (VersionCommand) {
		IConsoleManager::Get().UnregisterConsoleObject(VersionCommand);
		VersionCommand = nullptr;
	}
}

void FCommands::HandleReloadBrowserCommand(const TArray<FString> &Args) {
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Browser reloaded."));
	}

	FPlugin::ReloadWebBrowser();
}

void FCommands::HandleResetToFactoryDefaultsCommand(const TArray<FString> &Args) {
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Settings reset to factory defaults."));
	}

	FPlugin::ResetToFactoryDefaults();
}

void FCommands::HandleVersionCommand(const TArray<FString> &Args) {
	if (GEngine) {
		FString Version = FPlugin::GetVersion();
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Plugin version: %s"), *Version));
	}
}
