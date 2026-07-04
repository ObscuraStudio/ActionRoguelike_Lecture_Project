// Copyright 2024 Leartes Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Bridge.generated.h"

class UFileDownloadHandler;
class SWebBrowser;
class FTempAssetHandler;

// Bridge class for handling various operations.
UCLASS()
class UBridge : public UObject {
	GENERATED_BODY()

public:
	static UBridge *Bridge;
	
	// Handles the start of a drag operation.
	UFUNCTION()
	static bool DragStarted(const FString &ProductType, const FString &HTMLContent,const FString &ProductName, const FString &Hash, const FString &InLOD, const FString &InTargetAssetName,const FString &ObjectType,bool IsInteractive);

	// Downloads a zip file.
	UFUNCTION()
	static void DownloadZipFile(const FString &ID, const FString &Hash, const FString &URL, const FString &Title);

	// Downloads a zip file.
	UFUNCTION()
	static void CancelDownload(const FString &ID, const FString &Hash);

	// Unzips a downloaded product.
	static void UnzipDownloadedProduct(const FString &ZipFilePath, const FString &OutputFolderPath, bool bSkipFirstFolder);

	// Executes a SQL query.
	UFUNCTION()
	static FString Execute(const FString &SQLQuery);

	// Selects data from the database.
	UFUNCTION()
	static FString Select(const FString &SQLQuery);

	// Write logs from webbrowser.
	UFUNCTION()
	static void Log(const FString &LText);

	// Imports a product to the project.
	UFUNCTION()
	static bool ImportProductToProject(const FString &InAssetType, const FString &InProductName, const FString &Hash, bool OpenInContentDrawer, const FString &ObjectType,bool IsInteractive);

	// Get imported inner folder name.
	UFUNCTION()
	static FString GetInnerFolderName(const FString &InProductName, const FString &Hash, const FString &Type);

	// Get current rocket version.
	UFUNCTION()
	static FString GetVersion();

	// Reset to factory defaults.
	UFUNCTION()
	static void ResetFactoryDefaults();

	// Make visible tab after load.
	UFUNCTION()
	static void MakeBrowserVisible();

	// Reload web browser
	UFUNCTION()
	static void ReloadWebBrowser();

	// Open url in default browser.
	UFUNCTION()
	static void OpenURLInDefaultBrowser(const FString &URL);

	// Extract downloaded content to folder.
	UFUNCTION()
	static void ExtractToFolder(const FString &Hash, bool bOpenInExplorer);

	// Change default extraction path.
	UFUNCTION()
	static FString ChangeDefaultExtractionPath();

	// Change default extraction path.
	UFUNCTION()
	static FString GetOSName();

	// Change default extraction path.
	UFUNCTION()
	static void ExploreFolder(const FString &InDirectory);

	// Delete folders by hash
	UFUNCTION()
	static bool DeleteProduct(const FString &Hash);

	// Create environment with given zip.
	UFUNCTION()
	static FString CreateEnvironment(const FString &EnvironmentName);

	// Create environment with given zip.
	UFUNCTION()
	static bool ImportEnvironment(const FString &EnvironmentName);

	// Check if the environment imported before.
	UFUNCTION()
	static bool IsTheEnvironmentImported(const FString &EnvironmentName);
};
