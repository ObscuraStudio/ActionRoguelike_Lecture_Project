// Copyright 2024 Leartes Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

// Defines the name of the Rocket module.
#define R_MODULE_NAME TEXT("Rocket")

// Declares a log category for the Rocket module.
DECLARE_LOG_CATEGORY_EXTERN(LogRocket, Log, All);

// IRocketModule is an interface for the Rocket module.
class IRocketModule : public IModuleInterface {
public:
	// Returns a reference to the Rocket module.s
	static IRocketModule &Get() {
		return FModuleManager::LoadModuleChecked<IRocketModule>(R_MODULE_NAME);
	}

	// Checks if the Rocket module is available.
	static bool IsAvailable() {
		return FModuleManager::Get().IsModuleLoaded(R_MODULE_NAME);
	}
};
