// Copyright 2024 Leartes Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class FSourceManager {
public:
	static void InitializeSourceDirectory();
	static void DeleteRocketFolder();
	static bool DeleteFileInPaths(const FString &Hash);
};
