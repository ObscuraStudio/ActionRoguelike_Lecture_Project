// Copyright 2024 Leartes Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class FExplorer {
public:
	static void ExtractToFolder(const FString &Hash, bool bOpenInExplorer = false);
	static FString ChangeDefaultExtractionPath();

private:
	static FString GetExtractionPathFromDatabase();
	static bool ShowSaveFileDialog(const void *ParentWindowHandle, const FString &Title, const FString &DefaultPath, const FString &DefaultFile, TArray<FString> &OutFilenames);
	static bool ShowOpenDirectoryDialog(const void *ParentWindowHandle, const FString &Title, const FString &DefaultPath, FString &OutFolder);
};
