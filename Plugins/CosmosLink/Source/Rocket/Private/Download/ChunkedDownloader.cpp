// Copyright 2024 Leartes Studios. All Rights Reserved.

#include "ChunkedDownloader.h"
#include "HttpModule.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Async/Async.h"
#include "Interfaces/IHttpResponse.h"
#include "Downloader.h"
#include "FileDownloadHandler.h"
#include "Notification.h"
#include "PathHelpers.h"
#include "Rocket.h"
#include "RocketModule.h"
#include "SWebBrowser.h"

constexpr int64 DefaultChunkSize = 100 * 1024 * 1024;
constexpr int32 MaxRetryCount = 3;

TMap<FString, TSharedPtr<IHttpRequest, ESPMode::ThreadSafe>> FChunkedDownloader::ActiveRequests;

void FChunkedDownloader::DownloadFileInChunks(const FString& ID, const FString& Hash, const FString& URL) {
    FString FilePath = FPaths::Combine(FPathHelpers::GetRocketCompressedPath(), Hash + TEXT(".zip"));
    FChunkDownloadContext Context(ID, Hash, URL, FilePath,DefaultChunkSize);
	
    const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> InitialRequest = FHttpModule::Get().CreateRequest();
    InitialRequest->SetURL(URL);
    InitialRequest->SetVerb("GET");
    InitialRequest->SetHeader(TEXT("Range"), TEXT("bytes=0-0"));
    InitialRequest->OnProcessRequestComplete().BindStatic(&FChunkedDownloader::OnInitialGetRequestComplete, Context);
    if (!InitialRequest->ProcessRequest()) {
        UE_LOG(LogRocket, Error, TEXT("Failed to process initial GET request for %s"), *URL);
    }
}

void FChunkedDownloader::OnInitialGetRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FChunkDownloadContext Context) {
    if (bWasSuccessful && Response.IsValid()) {
        const FString ContentRangeStr = Response->GetHeader("Content-Range");
        if (!ContentRangeStr.IsEmpty()) {
            int64 TotalSize = -1;
            const int32 SlashIndex = ContentRangeStr.Find(TEXT("/"));
            if (SlashIndex != INDEX_NONE) {
                const FString TotalSizeStr = ContentRangeStr.Mid(SlashIndex + 1);
                TotalSize = FCString::Atoi64(*TotalSizeStr);
            }
            if (TotalSize > 0) {
                Context.TotalSize = TotalSize;
            } else {
                UE_LOG(LogRocket, Warning, TEXT("Failed to parse total size from Content-Range header for %s"), *Context.URL);
                Context.TotalSize = 0; // Content size unknown
            }
        } else {
            UE_LOG(LogRocket, Warning, TEXT("Content-Range header is missing or empty for %s"), *Context.URL);
            Context.TotalSize = 0; // Content size unknown
        }
        DownloadNextChunk(Context);
    } else {
        UE_LOG(LogRocket, Error, TEXT("Failed to get content size from %s"), *Context.URL);
        Context.TotalSize = 0; // Content size unknown
        DownloadNextChunk(Context);
    }
}

void FChunkedDownloader::DownloadNextChunk(FChunkDownloadContext& Context) {
    int64 EndOffset = (Context.TotalSize > 0)
        ? FMath::Min(Context.CurrentOffset + Context.ChunkSize - 1, Context.TotalSize - 1)
        : Context.CurrentOffset + Context.ChunkSize - 1;

    if (Context.TotalSize > 0 && Context.CurrentOffset >= Context.TotalSize) {
        FDownloadContext DownloadContext;
        DownloadContext.ID = Context.ID;
        DownloadContext.Hash = Context.Hash;
        AsyncTask(ENamedThreads::GameThread, [DownloadContext]() {
            FDownloader::OnDownloadComplete(true, DownloadContext);
        });
        return;
    }
	
    const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetURL(Context.URL);
    HttpRequest->SetVerb("GET");
    HttpRequest->SetHeader(TEXT("Range"), FString::Printf(TEXT("bytes=%lld-%lld"), Context.CurrentOffset, EndOffset));

#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4)
    HttpRequest->OnRequestProgress64().BindStatic(&FChunkedDownloader::OnRequestProgress64, Context);
#else
    HttpRequest->OnRequestProgress().BindStatic(&FChunkedDownloader::OnRequestProgress, Context);
#endif

    HttpRequest->OnProcessRequestComplete().BindLambda([Context](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) mutable {
        if (ActiveRequests.Contains(Context.Hash)) {
            ActiveRequests.Remove(Context.Hash);
            if (bWasSuccessful && Response.IsValid() && Response->GetContentLength() > 0) {
                const TArray<uint8>& ChunkData = Response->GetContent();
                if (ChunkData.Num() > 0) {
                    const FString ContentRangeStr = Response->GetHeader("Content-Range");
                    if (!ContentRangeStr.IsEmpty() && Context.TotalSize <= 0) {
                        int64 TotalSize = -1;
                        const int32 SlashIndex = ContentRangeStr.Find(TEXT("/"));
                        if (SlashIndex != INDEX_NONE) {
                            const FString TotalSizeStr = ContentRangeStr.Mid(SlashIndex + 1);
                            TotalSize = FCString::Atoi64(*TotalSizeStr);
                        }
                        if (TotalSize > 0) {
                            Context.TotalSize = TotalSize;
                        } else {
                            UE_LOG(LogRocket, Warning, TEXT("Failed to parse total size from Content-Range header for %s"), *Context.URL);
                        }
                    }

                    if (ChunkData.Num() < Context.ChunkSize / 2) {
                        Context.RetryCount++;
                        if (Context.RetryCount >= MaxRetryCount) {
                            FDownloadContext DownloadContext;
                            DownloadContext.ID = Context.ID;
                            DownloadContext.Hash = Context.Hash;
                            AsyncTask(ENamedThreads::GameThread, [DownloadContext, Request, Response]() {
                                FDownloader::OnDownloadComplete(true, DownloadContext);
                            });
                            return;
                        }
                    } else {
                        Context.RetryCount = 0; 
                    }

                    if (FFileHelper::SaveArrayToFile(ChunkData, *Context.FilePath, &IFileManager::Get(), FILEWRITE_Append)) {
                        Context.CurrentOffset += ChunkData.Num();
                    	DownloadNextChunk(Context);
                    } else {
                        UE_LOG(LogRocket, Error, TEXT("Failed to save chunk data to %s"), *Context.FilePath);
                        FDownloadContext DownloadContext;
                        DownloadContext.ID = Context.ID;
                        DownloadContext.Hash = Context.Hash;
                        AsyncTask(ENamedThreads::GameThread, [DownloadContext, Request, Response]() {
                            FDownloader::OnDownloadComplete(false, DownloadContext);
                        });
                    }
                } else {
                    UE_LOG(LogRocket, Error, TEXT("Received empty chunk data from %s"), *Context.URL);
                    FDownloadContext DownloadContext;
                    DownloadContext.ID = Context.ID;
                    DownloadContext.Hash = Context.Hash;
                    AsyncTask(ENamedThreads::GameThread, [DownloadContext, Request, Response]() {
                        FDownloader::OnDownloadComplete(false, DownloadContext);
                    });
                }
            } else {
                UE_LOG(LogRocket, Error, TEXT("Failed to download chunk from %s"), *Context.URL);
                FDownloadContext DownloadContext;
                DownloadContext.ID = Context.ID;
                DownloadContext.Hash = Context.Hash;
                AsyncTask(ENamedThreads::GameThread, [DownloadContext]() {
                    FDownloader::OnDownloadComplete(false, DownloadContext);
                });
            }
        }
    });
    ActiveRequests.Add(Context.Hash, HttpRequest);
    if (!HttpRequest->ProcessRequest()) {
        UE_LOG(LogRocket, Error, TEXT("Failed to process GET request for chunk %lld-%lld for %s"), Context.CurrentOffset, EndOffset, *Context.URL);
    }
}

#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4)
void FChunkedDownloader::OnRequestProgress64(FHttpRequestPtr Request, uint64 BytesSent, uint64 BytesReceived, FChunkDownloadContext Context) {
    if (!ActiveRequests.Contains(Context.Hash)) {
        return;
    }
    const int64 Progress = Context.CurrentOffset + BytesReceived;
    const int64 Total = (Context.TotalSize != 0) ? Context.TotalSize : Progress;

    const float ProgressPercentage = (Total > 0) ? (static_cast<float>(Progress) / Total) * 100.0f : 0.0f;
	
    AsyncTask(ENamedThreads::GameThread, [Context, Progress, ProgressPercentage]() mutable {
        FNotification::UpdateDownloadNotification(Context.Hash, ProgressPercentage);
        FDownloader::ReportDownloadProgress(Progress, Context.TotalSize, Context);
    });
}
#else
void FChunkedDownloader::OnRequestProgress(FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived, FChunkDownloadContext Context) {
    if (!ActiveRequests.Contains(Context.Hash)) {
        return;
    }
    const int64 Progress = Context.CurrentOffset + BytesReceived;
    const int64 Total = (Context.TotalSize != 0) ? Context.TotalSize : Progress;

    const float ProgressPercentage = (Total > 0) ? (static_cast<float>(Progress) / Total) * 100.0f : 0.0f;
	
    AsyncTask(ENamedThreads::GameThread, [Context, Progress, ProgressPercentage]() mutable {
        FNotification::UpdateDownloadNotification(Context.Hash, ProgressPercentage);
        FDownloader::ReportDownloadProgress(Progress, Context.TotalSize, Context);
    });
}
#endif

void FChunkedDownloader::CancelDownload(const FString& ID, const FString& Hash) {
	if (ActiveRequests.Contains(Hash)) {
		TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request = ActiveRequests[Hash];
		if (Request.IsValid()) {
			Request->CancelRequest();
			ActiveRequests.Remove(Hash);
			AsyncTask(ENamedThreads::GameThread, [ID, Hash]() {
				OnDownloadCancelledOnGameThread(ID, Hash);
				UFileDownloadHandler::RemoveCancelledTempAssetsWithHash(Hash);
			});
		}
	}
}

void FChunkedDownloader::OnDownloadCancelledOnGameThread(const FString &ID, const FString &Hash) {
	const FString JSCode = FString::Printf(TEXT("window.dispatchEvent(new CustomEvent('ue:download', { detail: { id: '%s', hash: '%s', status: 'cancelled' } }));"), *ID, *Hash);
	if (FRocket::WebBrowserWidget.IsValid()) {
		FRocket::WebBrowserWidget->ExecuteJavascript(JSCode);
	}
}


