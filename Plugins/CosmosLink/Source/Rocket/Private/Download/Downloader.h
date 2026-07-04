// Copyright 2024 Leartes Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NetworkDataContext.h"

class FDownloader {
public:
	static void DownloadZipFile(const FString& ID, const FString& Hash, const FString& URL, const FString& Title);
	static void CancelDownload(const FString& ID, const FString& Hash);
	static void OnDownloadComplete(bool bWasSuccessful, FDownloadContext Context);
	static void ReportDownloadProgress(int64 Progress, int64 Total, const FChunkDownloadContext& InContext);

private:
	static void OnDownloadCompleteOnGameThread(bool InSuccess, const FString& ID, const FString& Hash);
};
