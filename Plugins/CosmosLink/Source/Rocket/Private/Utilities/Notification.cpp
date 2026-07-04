// Copyright 2024 Leartes Studios. All Rights Reserved.

#include "Notification.h"
#include "Downloader.h"
#include "Framework/Notifications/NotificationManager.h"

TMap<FString, TSharedPtr<SNotificationItem>> FNotification::NotificationsMap;

void FNotification::StartDownloadNotification(const FString &ID, const FString &Hash, const FString &Title) {
	FNotificationInfo Info(FText::FromString(Title));
	Info.bFireAndForget = false;
	Info.ExpireDuration = 0.0f;
	Info.bUseThrobber = true;

	Info.ButtonDetails.Add(FNotificationButtonInfo(FText::FromString("Cancel"), FText::GetEmpty(), FSimpleDelegate::CreateLambda([ID, Hash]() {
		OnCancelDownloadClicked(ID, Hash);
	})));

	if (const TSharedPtr<SNotificationItem> NotificationItem = FSlateNotificationManager::Get().AddNotification(Info); NotificationItem.IsValid()) {
		NotificationItem->SetCompletionState(SNotificationItem::CS_Pending);
		NotificationItem->SetSubText(FText::FromString(TEXT("Download Progress: 0%")));
		NotificationsMap.Add(Hash, NotificationItem);
	}
}

void FNotification::UpdateDownloadNotification(const FString &Hash, float Progress) {
	if (NotificationsMap.Contains(Hash)) {
		if (const TSharedPtr<SNotificationItem> NotificationItem = *NotificationsMap.Find(Hash); NotificationItem.IsValid()) {
			NotificationItem->SetSubText(FText::FromString(FString::Printf(TEXT("Download Progress: %d%%"), static_cast<int32>(Progress))));
		}
	}
}

void FNotification::CompleteDownloadNotification(const FString &Hash, bool bSuccess) {
	if (NotificationsMap.Contains(Hash)) {
		if (const TSharedPtr<SNotificationItem> NotificationItem = *NotificationsMap.Find(Hash); NotificationItem.IsValid()) {
			if (bSuccess) {
				NotificationItem->SetSubText(FText::FromString(TEXT("Download Complete")));
				NotificationItem->SetCompletionState(SNotificationItem::CS_Success);
			} else {
				NotificationItem->SetSubText(FText::FromString(TEXT("Download Failed")));
				NotificationItem->SetCompletionState(SNotificationItem::CS_Fail);
			}
			NotificationItem->ExpireAndFadeout();
			RemoveNotification(Hash);
		}
	}
}

bool FNotification::IsNotificationActive(const FString &Hash) {
	return NotificationsMap.Contains(Hash);
}

void FNotification::CancelDownloadNotification(const FString &Hash) {
	if (NotificationsMap.Contains(Hash)) {
		if (const TSharedPtr<SNotificationItem> NotificationItem = *NotificationsMap.Find(Hash); NotificationItem.IsValid()) {
			NotificationItem->SetSubText(FText::FromString(TEXT("Download Cancelled")));
			NotificationItem->SetCompletionState(SNotificationItem::CS_None);
			NotificationItem->SetExpireDuration(5.0f);
			NotificationItem->ExpireAndFadeout();
			RemoveNotification(Hash);
		}
	}
}

void FNotification::OnCancelDownloadClicked(const FString &ID, const FString &Hash) {
	FDownloader::CancelDownload(ID, Hash);
	CancelDownloadNotification(Hash);
}

void FNotification::RemoveNotification(const FString &Hash) {
	NotificationsMap.Remove(Hash);
}
