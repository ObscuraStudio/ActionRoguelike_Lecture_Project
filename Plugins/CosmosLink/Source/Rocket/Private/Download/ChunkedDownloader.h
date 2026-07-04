// Copyright 2024 Leartes Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NetworkDataContext.h"
#include "Interfaces/IHttpRequest.h"

class FChunkedDownloader {
public:
	static void DownloadFileInChunks(const FString& ID, const FString& Hash, const FString& URL);
	static void CancelDownload(const FString& ID, const FString& Hash);
	static void OnDownloadCancelledOnGameThread(const FString &ID, const FString &Hash);

#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4)
	static void OnRequestProgress64(FHttpRequestPtr Request, uint64 BytesSent, uint64 BytesReceived, FChunkDownloadContext Context);
#else
	static void OnRequestProgress(FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived, FChunkDownloadContext Context);
#endif

private:
	static void DownloadNextChunk(FChunkDownloadContext& Context);
	static void OnInitialGetRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FChunkDownloadContext Context);
	static TMap<FString, TSharedPtr<IHttpRequest, ESPMode::ThreadSafe>> ActiveRequests;
};
