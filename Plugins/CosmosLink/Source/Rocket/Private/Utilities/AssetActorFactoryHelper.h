// Copyright 2024 Leartes Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetRegistry/AssetData.h"

class UActorFactory;

class FAssetActorFactoryHelper {
public:
	static UActorFactory *GetActorFactoryForAsset(const FAssetData &AssetData);
};
