// Copyright 2024 Leartes Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/NetworkDataContext.h"
#include "UObject/Object.h"
#include "FileDownloadHandler.generated.h"

class FTempAssetHandler;
class UBridge;

UCLASS()
class ROCKET_API UFileDownloadHandler : public UObject {
	GENERATED_BODY()

	// Pointer to the singleton instance

public:
	static UFileDownloadHandler *Instance;

	// Gets the singleton instance
	static UFileDownloadHandler *Get();

	static void Cleanup();

	bool AddNewWebDragOperation(const FRocketProductData &InDragStartData);

	bool TryToImportProductToProject(const FString &InAssetType,const FString &InObjectType, const FString &InProductName, const FString &InHash,bool IsInteractive, bool OpenInContentDrawer);

	static FString CreateNewEnvironment(const FString &InEnvironmentName);
	
	bool ImportEnvironment(const FString &InEnvironmentName) const;

	void ProceedImportEnvironment(bool bIsSuggestingRestart, const FString &InEnvironmentName) const;
	
	TMap<FString, TSharedPtr<FTempAssetHandler>> TempAssetData;

	FORCEINLINE bool IsDragging() const {
		return bIsDragging;
	}

	FORCEINLINE void SetIsDragging(bool bInIsDragging) {
		bIsDragging = bInIsDragging;
	}

private:
	void OnPluginsActivationRespondedForProduct(const TArray<struct FPluginViewItemInfo>& InPluginsInfo, const FName& InProductName, bool InApproved) const;
	void OnPluginsActivationRespondedForEnvironment(const TArray<struct FPluginViewItemInfo>& InPluginsInfo, const FName& InEnvironmentName, bool InApproved) const;

	
	void CreateOnActorsDroppedEvent();

	void DestroyOnActorsDroppedEvent() const;

	void OnActorsDropped(const TArray<UObject *> &InObjects, const TArray<AActor *> &InActors);

	// Calls after OnActorsDropped
	void OnRocketAssetDropped(const FPointerEvent &MouseEvent);

	void ReplaceTempAssetsWithProcessedAsset(FTempAssetHandler *InTempAssetHandler);

	void ImportCompleted(const FString &InProductName, const FString &Hash, const ERocketProductType &Type);

	bool bIsDragging = false;

	FAssetData DraggedAssetData;

	TSharedPtr<FTempAssetHandler> DraggingTempAssetHandler;

public:
	// Creates temporary asset data.
	static void CreateTempAssetData();

	static bool IsProductUAsset(const ERocketProductType &InType);

	void OnAssetProcessingCompleted(const FString &InHash);

	static void RemoveCancelledTempAssetsWithHash(const FString &InHash);

private:
	void CleanDragData();
};
