// Copyright 2024 Leartes Studios. All Rights Reserved.

#include "AssetActorFactoryHelper.h"
#include "ActorFactories/ActorFactory.h"
#include "ActorFactoryNiagara.h"
#include "Editor.h"
#include "NiagaraSystem.h"
#include "RocketModule.h"
#include "ActorFactories/ActorFactoryAmbientSound.h"
#include "ActorFactories/ActorFactoryBlueprint.h"
#include "ActorFactories/ActorFactoryDeferredDecal.h"
#include "ActorFactories/ActorFactorySkeletalMesh.h"
#include "ActorFactories/ActorFactoryStaticMesh.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Blueprint.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Sound/SoundWave.h"

// Helper function to check if the asset is a deferred decal material
bool IsDeferredDecalMaterial(const FAssetData& AssetData)
{
	// Check if the asset is a material
	if (!AssetData.IsInstanceOf(UMaterialInterface::StaticClass()))
	{
		return false; // Not a material, so cannot be a decal material
	}

	// Get the AssetRegistry module
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	uint32 SanityCheck = 0;
	FAssetData CurrentAssetData = AssetData;

	// Traverse parent materials if it is a UMaterialInstance
	while (SanityCheck < 1000 && !CurrentAssetData.IsInstanceOf(UMaterial::StaticClass()))
	{
		const FString ParentObjectPath = CurrentAssetData.GetTagValueRef<FString>("Parent");
		
		// If the parent path is missing, log it and exit
		if (ParentObjectPath.IsEmpty())
		{
			return false;
		}

		#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1)
		CurrentAssetData = AssetRegistry.GetAssetByObjectPath(FSoftObjectPath(ParentObjectPath));
		#else
		CurrentAssetData = AssetRegistry.GetAssetByObjectPath(FName(*ParentObjectPath));
		#endif

		// Load parent material manually
		UMaterial* ParentMaterial = LoadObject<UMaterial>(nullptr, *ParentObjectPath);
		if (!ParentMaterial)
		{
			UE_LOG(LogRocket, Warning, TEXT("Failed to load parent material: %s"), *ParentObjectPath);
			return false;
		}

		// Log if asset data is invalid
		if (!CurrentAssetData.IsValid())
		{
			UE_LOG(LogRocket, Log, TEXT("Invalid asset data for parent material."));
			return false;
		}

		++SanityCheck;
	}

	// Sanity check limit exceeded
	if (SanityCheck >= 1000)
	{
		UE_LOG(LogRocket, Log, TEXT("Exceeded sanity check limit while traversing parent materials."));
		return false; 
	}

	// If the asset is not a base material
	if (!CurrentAssetData.IsInstanceOf(UMaterial::StaticClass()))
	{
		UE_LOG(LogRocket, Log, TEXT("Parent material is not a valid base material."));
		return false;
	}

	const FString MaterialDomain = CurrentAssetData.GetTagValueRef<FString>("MaterialDomain");
	return MaterialDomain == TEXT("MD_DeferredDecal");
}


UActorFactory *FAssetActorFactoryHelper::GetActorFactoryForAsset(const FAssetData &AssetData) {

	if (!AssetData.IsValid()) {
		return nullptr;
	}
	
	if (AssetData.IsInstanceOf(UStaticMesh::StaticClass())) {
		return GEditor->FindActorFactoryByClass(UActorFactoryStaticMesh::StaticClass());
	}
	
	// Check if it's a decal material
	if (IsDeferredDecalMaterial(AssetData))
	{
		return GEditor->FindActorFactoryByClass(UActorFactoryDeferredDecal::StaticClass());
	}

	if (AssetData.IsInstanceOf(USkeletalMesh::StaticClass())) {
		return GEditor->FindActorFactoryByClass(UActorFactorySkeletalMesh::StaticClass());
	}

	if (AssetData.IsInstanceOf(UNiagaraSystem::StaticClass())) {
		return GEditor->FindActorFactoryByClass(UActorFactoryNiagara::StaticClass());
	}

	if (AssetData.IsInstanceOf(USoundWave::StaticClass())) {
		return GEditor->FindActorFactoryByClass(UActorFactoryAmbientSound::StaticClass());
	}

	if (AssetData.IsInstanceOf(UBlueprint::StaticClass())) {
		const UBlueprint *BlueprintAsset = Cast<UBlueprint>(AssetData.GetAsset());
		if (BlueprintAsset && BlueprintAsset->GeneratedClass->IsChildOf(AActor::StaticClass())) {
			return GEditor->FindActorFactoryByClass(UActorFactoryBlueprint::StaticClass());
		}
	}
	return nullptr;
}
