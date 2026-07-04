// Copyright 2024 Leartes Studios. All Rights Reserved.

#include "Downloader.h"
#include "Archiver.h"
#include "ChunkedDownloader.h"
#include "PathHelpers.h"
#include "Rocket.h"
#include "RocketModule.h"
#include "SWebBrowser.h"
#include "Async/Async.h"
#include "Misc/Paths.h"
#include "Utilities/Notification.h"

void FDownloader::DownloadZipFile(const FString& ID, const FString& Hash, const FString& URL, const FString& Title) {
    const FString ZipFilePath = FPaths::Combine(FPathHelpers::GetRocketCompressedPath(), Hash + TEXT(".zip"));
	
    if (FPaths::FileExists(ZipFilePath)) {
        AsyncTask(ENamedThreads::GameThread, [ID, Hash]() {
            OnDownloadCompleteOnGameThread(true, ID, Hash);
        });
        return;
    }

    FNotification::StartDownloadNotification(ID, Hash, Title);

    FChunkedDownloader::DownloadFileInChunks(ID, Hash, URL);
}

void FDownloader::CancelDownload(const FString& ID, const FString& Hash) {
    FChunkedDownloader::CancelDownload(ID, Hash);
}

void FDownloader::OnDownloadComplete(bool bWasSuccessful, FDownloadContext Context) {

	if (bWasSuccessful) {

		const FString FilePath = FPaths::Combine(FPathHelpers::GetRocketDepotPath(), TEXT("compressed"), Context.Hash + TEXT(".zip"));
		FArchiver::UnzipFile(FilePath, FPaths::Combine(FPathHelpers::GetRocketDepotPath(), TEXT("products"), Context.Hash), false);

		AsyncTask(ENamedThreads::GameThread, [Context]() {
			FNotification::CompleteDownloadNotification(Context.Hash, true);
			OnDownloadCompleteOnGameThread(true, Context.ID, Context.Hash);
		});
	} else {
		AsyncTask(ENamedThreads::GameThread, [Context]() {
			FNotification::CompleteDownloadNotification(Context.Hash, false);
			OnDownloadCompleteOnGameThread(false, Context.ID, Context.Hash);
		});
	
	}
}

void FDownloader::OnDownloadCompleteOnGameThread(bool bSuccess, const FString& ID, const FString& Hash) {
    
    FString EscapedID = ID;
    FString EscapedHash = Hash;
    EscapedID.ReplaceInline(TEXT("'"), TEXT("\\'"));
    EscapedHash.ReplaceInline(TEXT("'"), TEXT("\\'"));

    const FString Status = bSuccess ? TEXT("downloaded") : TEXT("error");
    const FString JSCode = FString::Printf(
        TEXT("window.dispatchEvent(new CustomEvent('ue:download', { detail: { id: '%s', hash: '%s', status: '%s' } }));"),
        *EscapedID, *EscapedHash, *Status
    );
	
    if (FRocket::WebBrowserWidget.IsValid()) {
        FRocket::WebBrowserWidget->ExecuteJavascript(JSCode);
    }
}

void FDownloader::ReportDownloadProgress(int64 Progress, int64 Total, const FChunkDownloadContext& Context) {
    
    FString EscapedID = Context.ID;
    FString EscapedHash = Context.Hash;
    EscapedID.ReplaceInline(TEXT("'"), TEXT("\\'"));
    EscapedHash.ReplaceInline(TEXT("'"), TEXT("\\'"));

    const float ProgressMB = static_cast<float>(Progress) / (1024.0f * 1024.0f);
    const float TotalMB = static_cast<float>(Total) / (1024.0f * 1024.0f);

    const FString JSCode = FString::Printf(
        TEXT("window.dispatchEvent(new CustomEvent('ue:download', { detail: { id: '%s', hash: '%s', status: 'downloading', progress: %f, total: %f } }));"),
        *EscapedID, *EscapedHash, ProgressMB, TotalMB
    );

    if (FRocket::WebBrowserWidget.IsValid()) {
        FRocket::WebBrowserWidget->ExecuteJavascript(JSCode);
    } else {
        UE_LOG(LogRocket, Warning, TEXT("FRocket::WebBrowserWidget is not valid"));
    }
}
