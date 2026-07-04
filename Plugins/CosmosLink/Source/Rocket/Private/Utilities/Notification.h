// Copyright 2024 Leartes Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Notifications/SNotificationList.h"

class FNotification {
public:
	static void StartDownloadNotification(const FString &ID, const FString &Hash, const FString &Title);
	static void UpdateDownloadNotification(const FString &Hash, float Progress);
	static void CompleteDownloadNotification(const FString &Hash, bool bSuccess);
	static bool IsNotificationActive(const FString &Hash);
	static void CancelDownloadNotification(const FString &Hash);

private:
	static TMap<FString, TSharedPtr<SNotificationItem>> NotificationsMap;
	static void OnCancelDownloadClicked(const FString &ID, const FString &Hash);
	static void RemoveNotification(const FString &Hash);
};
