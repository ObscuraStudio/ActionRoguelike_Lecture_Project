// Copyright 2024 Leartes Studios. All Rights Reserved.

#include "Bridge.h"
#include "Archiver.h"
#include "AssetUtilities.h"
#include "Browser.h"
#include "Downloader.h"
#include "Editor.h"
#include "Explorer.h"
#include "FileDownloadHandler.h"
#include "NetworkDataContext.h"
#include "PlatformHelpers.h"
#include "Plugin.h"
#include "RocketModule.h"
#include "SourceManager.h"
#include "Engine/StaticMesh.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Paths.h"
#include "Utilities/Database.h"
#include "Utilities/PathHelpers.h"

UBridge *UBridge::Bridge = nullptr;

#pragma region Import

bool UBridge::DragStarted(const FString &ProductType, const FString &HTMLContent, const FString &ProductName, const FString &Hash, const FString &InLOD, const FString &InTargetAssetName, const FString &ObjectType,bool IsInteractive) {
	if (!IsValid(Bridge)) {
		return false;
	}
	
	FRocketProductData DragData(ProductType,ObjectType, HTMLContent, ProductName, Hash, InLOD, InTargetAssetName,IsInteractive);

	return UFileDownloadHandler::Get()->AddNewWebDragOperation(DragData);
}

bool UBridge::ImportProductToProject(const FString &InAssetType, const FString &InProductName, const FString &Hash, bool OpenInContentDrawer, const FString &ObjectType,bool IsInteractive) {
	if (InProductName.IsEmpty()) {
		return false;
	}
	return UFileDownloadHandler::Get()->TryToImportProductToProject(InAssetType,ObjectType, InProductName, Hash,IsInteractive, OpenInContentDrawer);
}

FString UBridge::GetInnerFolderName(const FString &InProductName, const FString &Hash, const FString &Type) {
	FString MainFolderName = FPathHelpers::FindMainFolderNameForProduct(InProductName, Hash, FAssetUtilities::GetAssetTypeFromString(Type));
	return MainFolderName.IsEmpty() ? InProductName : MainFolderName;
}

FString UBridge::CreateEnvironment(const FString &EnvironmentName) {
	return UFileDownloadHandler::CreateNewEnvironment(EnvironmentName);
}

bool UBridge::ImportEnvironment(const FString &EnvironmentName) {
	return UFileDownloadHandler::Get()->ImportEnvironment(EnvironmentName);
}

bool UBridge::IsTheEnvironmentImported(const FString &EnvironmentName) {
	return FPathHelpers::IsAnyEnvSourceFileImportedBefore(EnvironmentName);
}

#pragma endregion Import

#pragma region Download


void UBridge::DownloadZipFile(const FString &ID, const FString &Hash, const FString &URL, const FString &Title) {
	FDownloader::DownloadZipFile(ID, Hash, URL, Title);
}

void UBridge::CancelDownload(const FString &ID, const FString &Hash) {
	FDownloader::CancelDownload(ID, Hash);
}

void UBridge::UnzipDownloadedProduct(const FString &ZipFilePath, const FString &OutputFolderPath, bool bSkipFirstFolder) {
	FArchiver::UnzipFile(ZipFilePath, OutputFolderPath, bSkipFirstFolder);
}

#pragma endregion Download

#pragma region Database

FString UBridge::Execute(const FString &SQLQuery) {
	return FDatabase::Execute(SQLQuery);
}

FString UBridge::Select(const FString &SQLQuery) {
	return FDatabase::Select(SQLQuery);
}
#pragma endregion Database

#pragma region Log

void UBridge::Log(const FString &LText) {
	UE_LOG(LogRocket, Log, TEXT("%s"), *LText);
}

#pragma endregion Log

#pragma region Plugin

FString UBridge::GetVersion() {
	return FPlugin::GetVersion();
}

void UBridge::ResetFactoryDefaults() {
	return FPlugin::ResetToFactoryDefaults();
}

void UBridge::MakeBrowserVisible() {
	FPlugin::MakeBrowserVisible();
}

void UBridge::ReloadWebBrowser() {
	FPlugin::ReloadWebBrowser();
}

#pragma endregion Plugin

#pragma region Browser

void UBridge::OpenURLInDefaultBrowser(const FString &URL) {
	FBrowser::OpenURLInDefaultBrowser(URL);
}

#pragma endregion Browser

#pragma region Explorer

void UBridge::ExtractToFolder(const FString &Hash, bool bOpenInExplorer) {
	FExplorer::ExtractToFolder(Hash, bOpenInExplorer);
}

FString UBridge::ChangeDefaultExtractionPath() {
	return FExplorer::ChangeDefaultExtractionPath();
}

FString UBridge::GetOSName() {
	return FRocketPlatformHelpers::GetPlatformName();
}

void UBridge::ExploreFolder(const FString &InDirectory) {
	return FPlatformProcess::ExploreFolder(*InDirectory);
}

bool UBridge::DeleteProduct(const FString &Hash) {
	return FSourceManager::DeleteFileInPaths(Hash);
}

#pragma endregion Explorer
