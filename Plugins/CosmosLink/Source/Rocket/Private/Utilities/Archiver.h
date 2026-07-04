// Copyright 2024 Leartes Studios. All Rights Reserved.

#pragma once

#include <miniz.h>

#include "CoreMinimal.h"

class FArchiver {
public:
	// Unzips a zip file.
	static void UnzipFile(const FString &ZipFilePath, const FString &OutputFolderPath, bool bSkipFirstFolder);
};
