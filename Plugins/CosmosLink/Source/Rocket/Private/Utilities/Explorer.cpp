// Copyright 2024 Leartes Studios. All Rights Reserved.

#include "Explorer.h"
#include "Archiver.h"
#include "Database.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "RocketModule.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FeedbackContext.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Utilities/PathHelpers.h"

FString FExplorer::GetExtractionPathFromDatabase() {
	const FString SQLQuery = TEXT("SELECT value FROM settings WHERE key = 'extractionPath'");
	const FString ExtractionPathJson = FDatabase::Select(SQLQuery);

	FString ExtractionPath;
	TArray<TSharedPtr<FJsonValue>> JsonValues;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ExtractionPathJson);
	if (FJsonSerializer::Deserialize(Reader, JsonValues) && JsonValues.Num() > 0) {
		const TSharedPtr<FJsonObject> JsonObject = JsonValues[0]->AsObject();
		ExtractionPath = JsonObject->GetStringField(TEXT("value"));
	}

	if (ExtractionPath.IsEmpty()) {
		UE_LOG(LogRocket, Warning, TEXT("No extractionPath found in settings, using default path."));
		ExtractionPath = FPathHelpers::GetDefaultRocketExportPath();
	}

	return ExtractionPath;
}

bool FExplorer::ShowSaveFileDialog(const void *ParentWindowHandle, const FString &Title, const FString &DefaultPath, const FString &DefaultFile, TArray<FString> &OutFilenames) {
	IDesktopPlatform *DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform) {
		UE_LOG(LogRocket, Error, TEXT("Failed to get DesktopPlatform"));
		return false;
	}

	return DesktopPlatform->SaveFileDialog(ParentWindowHandle, Title, DefaultPath, DefaultFile, TEXT("All Files (*.*)|*.*"), EFileDialogFlags::None, OutFilenames);
}

bool FExplorer::ShowOpenDirectoryDialog(const void *ParentWindowHandle, const FString &Title, const FString &DefaultPath, FString &OutFolder) {
	IDesktopPlatform *DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform) {
		UE_LOG(LogRocket, Error, TEXT("Failed to get DesktopPlatform"));
		return false;
	}

	return DesktopPlatform->OpenDirectoryDialog(ParentWindowHandle, Title, DefaultPath, OutFolder);
}

void FExplorer::ExtractToFolder(const FString &Hash, bool bOpenInExplorer) {
	if (Hash.IsEmpty()) {
		UE_LOG(LogRocket, Warning, TEXT("Hash is empty, cannot extract."));
		return;
	}

	const FString ZipFilePath = FPaths::Combine(FPathHelpers::GetRocketCompressedPath(), Hash + TEXT(".zip"));

	// Ensure the zip file exists
	if (!FPaths::FileExists(ZipFilePath)) {
		UE_LOG(LogRocket, Error, TEXT("Zip file does not exist: %s"), *ZipFilePath);
		return;
	}

	const void *ParentWindowHandle = nullptr;
	if (GEngine && GEngine->GameViewport) {
		ParentWindowHandle = GEngine->GameViewport->GetWindow()->GetNativeWindow()->GetOSWindowHandle();
	}

	const FString DefaultProjectPath = GetExtractionPathFromDatabase();
	const FString Title = TEXT("Enter folder name for extraction");
	const FString DefaultFile = Hash;

	TArray<FString> OutFilenames;
	if (ShowSaveFileDialog(ParentWindowHandle, Title, DefaultProjectPath, DefaultFile, OutFilenames)) {
		if (OutFilenames.Num() > 0) {
			const FString SelectedFolder = OutFilenames[0];
			FString EnteredFolderName = FPaths::GetBaseFilename(SelectedFolder);
			const FString TargetFolder = FPaths::Combine(FPaths::GetPath(SelectedFolder), EnteredFolderName);
			FString FullTargetFolder = FPaths::ConvertRelativePathToFull(TargetFolder);

			// Call the UnzipFile function from FArchiver
			FArchiver::UnzipFile(ZipFilePath, TargetFolder, false);

			if (bOpenInExplorer) {
				FPlatformProcess::ExploreFolder(*FullTargetFolder);
			}
		} else {
			UE_LOG(LogRocket, Log, TEXT("No file name entered"));
		}
	} else {
		//UE_LOG(LogRocket, Warning, TEXT("Dialog cancelled"));
	}
}

FString FExplorer::ChangeDefaultExtractionPath() {
	const void *ParentWindowHandle = nullptr;
	if (GEngine && GEngine->GameViewport) {
		ParentWindowHandle = GEngine->GameViewport->GetWindow()->GetNativeWindow()->GetOSWindowHandle();
	}

	const FString DefaultProjectPath = GetExtractionPathFromDatabase();
	const FString Title = TEXT("Select Extraction Path");

	FString OutFolder;
	if (ShowOpenDirectoryDialog(ParentWindowHandle, Title, DefaultProjectPath, OutFolder)) {
		if (!OutFolder.IsEmpty()) {
			FString SelectedFolder = OutFolder;
			FSlateApplication::Get().SetAllUserFocusToGameViewport();

			return SelectedFolder;
		}
	} else {
		//UE_LOG(LogRocket, Warning, TEXT("Dialog cancelled"));
	}

	FSlateApplication::Get().SetAllUserFocusToGameViewport();

	return FString();
}
