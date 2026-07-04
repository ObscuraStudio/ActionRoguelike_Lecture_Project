// Copyright 2024 Leartes Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

const FName InteractiveMetadataKey(TEXT("interactive"));

enum class ERocketProductType : uint8
{
	Asset,
	Environment,
	Material,
	Vfx,
	Sound,
	Tool,
	Bundle,
	Tutorial,
	Decal,
	Unknown
};

enum class ERocketObjectType : uint8
{
	Building,
	Character,
	Furniture,
	Foliage,
	Weapon,
	Vehicle,
	Prop,
	Unknown
};

struct FChunkDownloadContext {
	FString ID;
	FString Hash;
	FString URL;
	FString FilePath;
	int64 ChunkSize;
	int64 CurrentOffset;
	int64 TotalSize;
	int32 RetryCount;

	FChunkDownloadContext()
		: ChunkSize(0), CurrentOffset(0), TotalSize(-1), RetryCount(0) {
	}

	FChunkDownloadContext(const FString& InID, const FString& InHash, const FString& InURL, const FString& InFilePath, int64 InChunkSize)
		: ID(InID), Hash(InHash), URL(InURL), FilePath(InFilePath), ChunkSize(InChunkSize), CurrentOffset(0), TotalSize(-1), RetryCount(0) {
	}
};

struct FDownloadContext {
	FString ID;
	FString Hash;
	class UBridge* BridgeInstance;
};

struct FRocketProductData {
	FString AssetType;
	FString ObjectType;
	FString HTMLContent;
	FString ProductName;
	FString Hash;
	FString LOD;
	FString TargetAssetName;
	bool bIsInteractive;

	FRocketProductData()
		: AssetType(TEXT("")), ObjectType(TEXT("")), HTMLContent(TEXT("")), ProductName(TEXT("")), Hash(TEXT("")), LOD(TEXT("")), TargetAssetName(TEXT("")), bIsInteractive(false) {
	}

	FRocketProductData(const FString& InProductType,const FString& InObjectType, const FString& InHTMLContent, const FString& InProductName, const FString& InHash, const FString& InLOD, const FString& InTargetAssetName, bool IsInteractive)
		: AssetType(InProductType), ObjectType(InObjectType), HTMLContent(InHTMLContent), ProductName(InProductName), Hash(InHash), LOD(InLOD), TargetAssetName(InTargetAssetName), bIsInteractive(IsInteractive) {
	}

	FString ToString() const {
		return FString::Printf(TEXT("ProductType: %s,ObjectType:%s HTMLContent: %s, ProductName: %s, Hash: %s, LOD: %s, TargetAssetName: %s"), *AssetType,*ObjectType, *HTMLContent, *ProductName, *Hash, *LOD, *TargetAssetName);
	}
};


