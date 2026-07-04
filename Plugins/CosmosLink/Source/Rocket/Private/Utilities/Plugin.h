// Copyright 2024 Leartes Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class FPlugin {
public:
	static FString GetVersion();
	static void MakeBrowserVisible();
	static void ReloadWebBrowser();
	static void ResetToFactoryDefaults();
};
