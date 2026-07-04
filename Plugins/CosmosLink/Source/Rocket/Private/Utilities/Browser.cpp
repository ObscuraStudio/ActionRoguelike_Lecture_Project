// Copyright 2024 Leartes Studios. All Rights Reserved.

#include "Browser.h"
#include "RocketModule.h"
#include "HAL/PlatformProcess.h"
#include "Logging/LogMacros.h"
#include "Misc/Paths.h"

void FBrowser::OpenURLInDefaultBrowser(const FString &URL) {
	if (URL.IsEmpty()) {
		UE_LOG(LogRocket, Warning, TEXT("URL is empty, cannot open in browser."));
		return;
	}

	FPlatformProcess::LaunchURL(*URL, nullptr, nullptr);
}
