// Copyright 2024 Leartes Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Misc/CoreDelegates.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SWidget.h"
#include "Widgets/SWindow.h"

class UFileDownloadHandler;

class FUIHelpers {
public:
	static void ShowSoundSelectionWindow(UFileDownloadHandler *Handler, const TArray<FAssetData> &AssetDataArray, const FString &InProductName, const FString &Hash);
	static void CloseSoundSelectionWindow();
	static void OnSoundSelected(const TSharedPtr<FAssetData> &SelectedSound, ESelectInfo::Type SelectInfo, UFileDownloadHandler *Handler, const FString &InProductName, const FString &Hash);
	static FReply OnImportButtonClicked(UFileDownloadHandler *Handler, const FString &InProductName, const FString &Hash);

private:
	static TSharedPtr<SWindow> SoundSelectionWindow;
	static TArray<TSharedPtr<FAssetData>> SharedAssetDataArray;
	static TSharedPtr<FAssetData> SelectedSoundAsset;
};
