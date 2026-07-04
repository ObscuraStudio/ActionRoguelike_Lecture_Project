// Copyright Narrative Tools 2025. 

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

//Runtime module for narrative 
class FNarrativeModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
