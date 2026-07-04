// Copyright 2024 Leartes Studios. All Rights Reserved.

#include "SourceManager.h"
#include "PathHelpers.h"
#include "RocketModule.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Paths.h"

void FSourceManager::InitializeSourceDirectory() {
	// Get the Rocket depot path
	const FString RocketDepotPath = FPathHelpers::GetRocketDepotPath();

	// Check if the Rocket depot path exists, if not create it
	if (!IFileManager::Get().DirectoryExists(*RocketDepotPath)) {
		IFileManager::Get().MakeDirectory(*RocketDepotPath);
	}

	// Get the Rocket products path
	const FString RocketProductsPath = FPathHelpers::GetRocketProductsPath();

	// Check if the products folder exists, if not create it
	if (!IFileManager::Get().DirectoryExists(*RocketProductsPath)) {
		IFileManager::Get().MakeDirectory(*RocketProductsPath);
	}

	// Assuming the compressed folder is a subfolder in the depot path
	const FString CompressedFolderPath = FPathHelpers::GetRocketCompressedPath();

	// Check if the compressed folder exists, if not create it
	if (!IFileManager::Get().DirectoryExists(*CompressedFolderPath)) {
		IFileManager::Get().MakeDirectory(*CompressedFolderPath);
	}

	// Get the Rocket export products path
	const FString RocketExportPath = FPathHelpers::GetDefaultRocketExportPath();

	// Check if the export products folder exists, if not create it
	if (!IFileManager::Get().DirectoryExists(*RocketExportPath)) {
		IFileManager::Get().MakeDirectory(*RocketExportPath);
	}
}

void FSourceManager::DeleteRocketFolder() {
	// Define the path to the folder
	const FString FolderPath = FPaths::Combine(FPathHelpers::GetRocketDepotPath());

	// Ensure the folder exists
	if (FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*FolderPath)) {
		// Delete the folder and all its contents
		if (FPlatformFileManager::Get().GetPlatformFile().DeleteDirectoryRecursively(*FolderPath)) {
			//UE_LOG(LogRocket, Log, TEXT("Successfully deleted folder: %s"), *FolderPath);
		} else {
			UE_LOG(LogRocket, Error, TEXT("Failed to delete folder: %s"), *FolderPath);
		}
	} else {
		UE_LOG(LogRocket, Warning, TEXT("Folder does not exist: %s"), *FolderPath);
	}
}

bool FSourceManager::DeleteFileInPaths(const FString &FileName) {
	const FString CompressedFolderPath = FPathHelpers::GetRocketCompressedPath();
	const FString RocketProductsPath = FPathHelpers::GetRocketProductsPath();

	bool bAllDeleted = true;

	const FString ZipFilePath = FPaths::Combine(CompressedFolderPath, FileName + TEXT(".zip"));
	if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*ZipFilePath)) {
		if (FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*ZipFilePath)) {
			//UE_LOG(LogRocket, Log, TEXT("Successfully deleted file: %s"), *ZipFilePath);
		} else {
			UE_LOG(LogRocket, Error, TEXT("Failed to delete file: %s"), *ZipFilePath);
			bAllDeleted = false;
		}
	} else {
		UE_LOG(LogRocket, Warning, TEXT("File does not exist: %s"), *ZipFilePath);
		bAllDeleted = false;
	}

	const FString FolderPath = FPaths::Combine(RocketProductsPath, FileName);
	if (FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*FolderPath)) {
		if (FPlatformFileManager::Get().GetPlatformFile().DeleteDirectoryRecursively(*FolderPath)) {
			//UE_LOG(LogRocket, Log, TEXT("Successfully deleted folder: %s"), *FolderPath);
		} else {
			UE_LOG(LogRocket, Error, TEXT("Failed to delete folder: %s"), *FolderPath);
			bAllDeleted = false;
		}
	} else {
		UE_LOG(LogRocket, Warning, TEXT("Folder does not exist: %s"), *FolderPath);
		bAllDeleted = false;
	}

	return bAllDeleted;
}
