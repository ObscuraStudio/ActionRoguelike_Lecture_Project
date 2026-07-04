// Copyright 2024 Leartes Studios. All Rights Reserved.


#include "PlatformHelpers.h"

FString FRocketPlatformHelpers::GetPlatformName() {
#if PLATFORM_WINDOWS
	return TEXT("Windows");
#elif PLATFORM_LINUX
	return TEXT("Linux");
#elif PLATFORM_MAC
	return TEXT("Mac");
#else
	return TEXT("Unknown");
#endif
}