// Copyright Narrative Tools 2025. 

#include "Narrative.h"

#include "GameplayTagsManager.h"
#include "NarrativeGameplayTags.h"

#define LOCTEXT_NAMESPACE "FNarrativeModule"

void FNarrativeModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FNarrativeGameplayTags::Get().InitializeNativeTags();
	UGameplayTagsManager::Get().DoneAddingNativeTags(); 
	
}

void FNarrativeModule::ShutdownModule()
{
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FNarrativeModule, Narrative)
