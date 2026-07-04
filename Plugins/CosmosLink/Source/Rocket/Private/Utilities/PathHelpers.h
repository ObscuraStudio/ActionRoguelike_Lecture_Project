// Copyright 2024 Leartes Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NetworkDataContext.h"

// Handles Rocket system paths.
class FPathHelpers {
public:
	static FString GetHomeDirectory();
	// Returns the Rocket depot path.
	static FString GetRocketDepotPath();

	// Returns the Rocket products path.
	static FString GetRocketProductsPath();
	static FString GetDefaultRocketExportPath();
	static FString GetRocketCompressedPath();
	static FString GetDefaultProjectPath();

	// Return asset releated paths
	static bool DoesAssetDirectoryExist(const FString &InDirectory);
	static FString FindMainFolderNameForProduct(const FString &InProductsName, const FString &Hash, const ERocketProductType &Type = ERocketProductType::Unknown);
	static FString AssetDirectory(const FString &InProductsName, const FString &Hash, const ERocketProductType &InType);
	static FString CombineProductNameWithHash(const FString &InProductsName, const FString &Hash, const ERocketProductType &InType);
	static FString GetProductImportableContentPath(const FString &InProductsName, const FString &Hash, const ERocketProductType &Type);
	static TArray<FString> GetTopLevelFolders(const FString &ProductRocketPath);
	
	//Environment Checks
	static bool IsTheEnvironmentImportedBefore(const FString &InEnvironment);
	static bool IsAnyEnvSourceFileImportedBefore(const FString &InEnvironment);

};
