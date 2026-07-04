// Copyright 2024 Leartes Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NetworkDataContext.h"
#include "TempAssetHandler.h"


// FAssetUtilities class declaration
class FAssetUtilities {

public:
	//Calls on import completed
	static FAssetData TryToFindImportedDraggedAssetData(const TSharedPtr<FTempAssetHandler> &InDraggedAssetHandler);
	static bool TryToGetImportedExactProductData(const FRocketProductData &InProductData, const FString &InProductFolderName, FAssetData &OutAssetData);
	static bool TryToGetImportedExactProductMaterialData(const FString &ProductName, int32 Quality, FAssetData &OutAssetData);
	static void CreateSoundWaveAsset(const FString &SourceFolderPath, const FString &Folder);
	static void ApplyMaterialToComponent(AActor *Actor, const FString &MaterialPath, const FString &LOD);
	static int32 GetResolutionFromLOD(const FString &LOD);
	static ERocketProductType GetAssetTypeFromString(const FString &ProductTypeString);
	static ERocketObjectType GetObjectTypeFromString(const FString &ObjectTypeString);

private:
	static FString FindMatchingAssetName(const FString &InProductName, const FString &Hash, const FString &InLOD, const FString &InAssetPrefix, const ERocketProductType &Type);
	static FString FindMatchingMaterialName(const FString &ProductName, int32 Quality);

	static TMap<FName, FString> GetProductMetadata(const FAssetData& AssetData);
};
