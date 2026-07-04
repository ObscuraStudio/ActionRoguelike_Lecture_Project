// Copyright 2024 Leartes Studios. All Rights Reserved.

#include "Archiver.h"
#include "RocketModule.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

void FArchiver::UnzipFile(const FString &ZipFilePath, const FString &OutputFolderPath, bool bSkipFirstFolder) {
	if (!IFileManager::Get().MakeDirectory(*OutputFolderPath, true)) {
		UE_LOG(LogRocket, Error, TEXT("Failed to create output directory: %s"), *OutputFolderPath);
		return;
	}

	mz_zip_archive ZipArchive = {};

	if (!mz_zip_reader_init_file(&ZipArchive, TCHAR_TO_ANSI(*ZipFilePath), 0)) {
		UE_LOG(LogRocket, Error, TEXT("Failed to open zip file: %s"), *ZipFilePath);
		return;
	}

	const int FileCount = static_cast<int>(mz_zip_reader_get_num_files(&ZipArchive));
	FString FirstFolderName;
	bool FirstFolderNameSet = false;

	for (int i = 0; i < FileCount; i++) {
		mz_zip_archive_file_stat FileStat;
		if (!mz_zip_reader_file_stat(&ZipArchive, i, &FileStat)) {
			UE_LOG(LogRocket, Error, TEXT("Failed to get file stat for file index %d in zip file: %s"), i, *ZipFilePath);
			continue;
		}

		FString FilePath = ANSI_TO_TCHAR(FileStat.m_filename);
		FString TargetPath;

		if (bSkipFirstFolder) {
			if (!FirstFolderNameSet) {
				FirstFolderName = FPaths::GetPath(FilePath);
				FirstFolderNameSet = true;
			}

			FString RelativePath = FilePath.Replace(*FirstFolderName, TEXT(""));
			TargetPath = FPaths::Combine(OutputFolderPath, RelativePath);
		} else {
			TargetPath = FPaths::Combine(OutputFolderPath, FilePath);
		}

		if (mz_zip_reader_is_file_a_directory(&ZipArchive, i)) {
			if (!IFileManager::Get().MakeDirectory(*TargetPath, true)) {
				UE_LOG(LogRocket, Error, TEXT("Failed to create directory: %s"), *TargetPath);
			}
		} else {
			FString DirectoryPath = FPaths::GetPath(TargetPath);
			if (!IFileManager::Get().MakeDirectory(*DirectoryPath, true)) {
				UE_LOG(LogRocket, Error, TEXT("Failed to create directory: %s"), *DirectoryPath);
			}

			if (!mz_zip_reader_extract_to_file(&ZipArchive, i, TCHAR_TO_ANSI(*TargetPath), 0)) {
				UE_LOG(LogRocket, Error, TEXT("Failed to extract file: %s"), *TargetPath);
			}
		}
	}

	mz_zip_reader_end(&ZipArchive);
}
