// Copyright 2024 Leartes Studios. All Rights Reserved.

#include "FileDownloadHandler.h"
#include "Archiver.h"
#include "AssetActorFactoryHelper.h"
#include "AssetDragDrop.h"
#include "AssetUtilities.h"
#include "ContentBrowserModule.h"
#include "DesktopPlatformModule.h"
#include "IContentBrowserSingleton.h"
#include "IDesktopPlatform.h"
#include "NetworkDataContext.h"
#include "NiagaraActor.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "PathHelpers.h"
#include "DependencyManager.h"
#include "RequiredPluginsWindow.h"
#include "RocketModule.h"
#include "Selection.h"
#include "TempAssetHandler.h"
#include "ToolAssetData.h"
#include "TraceHelpers.h"
#include "UIHelpers.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Async/Async.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/Material.h"

#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3)
#include "ScreenPass.h"
#endif

UFileDownloadHandler *UFileDownloadHandler::Instance = nullptr;

UFileDownloadHandler *UFileDownloadHandler::Get() {
	if (!Instance) {
		Instance = NewObject<UFileDownloadHandler>();
		Instance->AddToRoot();
	}
	return Instance;
}

void UFileDownloadHandler::Cleanup() {
	if (IsValid(Instance)) {
		Instance->RemoveFromRoot();
		Instance = nullptr;
	}
}



bool UFileDownloadHandler::AddNewWebDragOperation(const FRocketProductData &InDragStartData) {
	if (InDragStartData.ProductName.IsEmpty() || InDragStartData.AssetType.IsEmpty()) {
		UE_LOG(LogRocket, Error, TEXT("Invalid product data, download is terminating..."));
		return false;
	}

	TArray<FString> FoundDependencies = FRocketDependencyManager::FindPluginDependencies(InDragStartData.AssetType,InDragStartData.ObjectType,InDragStartData.bIsInteractive);

	if(!FoundDependencies.IsEmpty()) {
		
		TArray<FString> MustActivateEnginePluginList;

		for (const FString& PluginName : FoundDependencies)
		{
			if (!FRocketDependencyManager::IsPluginEnabled(PluginName) && FRocketDependencyManager::CheckIfEnginePlugin(PluginName))
			{
				MustActivateEnginePluginList.Add(PluginName);
			}
		}
		if(!MustActivateEnginePluginList.IsEmpty()) {
			TArray<FPluginViewItemInfo> PluginViewInfo = FRocketDependencyManager::CreatePluginViewItemInfoArray(MustActivateEnginePluginList);
			FText Message = FText::FromName(TEXT("The following plugins need to be enabled to use this asset. Would you like to activate them now?"));
			FRocketDependencyUIManager::SpawnPluginEnableWindow(InDragStartData.ProductName, Message, PluginViewInfo, FOnPluginsActivationResponseSignature::CreateUObject(this, &UFileDownloadHandler::OnPluginsActivationRespondedForProduct));
			return false;
		}
	}
	
	const FString ProductFolderName = FPathHelpers::FindMainFolderNameForProduct(InDragStartData.ProductName, InDragStartData.Hash, FAssetUtilities::GetAssetTypeFromString(InDragStartData.AssetType));

	if (FAssetUtilities::GetAssetTypeFromString(InDragStartData.AssetType) == ERocketProductType::Material) {
		FAssetUtilities::TryToGetImportedExactProductMaterialData(InDragStartData.ProductName, FAssetUtilities::GetResolutionFromLOD(InDragStartData.LOD), DraggedAssetData);
		DraggingTempAssetHandler = MakeShareable(new FTempAssetHandler(nullptr, InDragStartData));

	} else if (FAssetUtilities::GetAssetTypeFromString(InDragStartData.AssetType) == ERocketProductType::Sound) {

		DraggingTempAssetHandler = MakeShareable(new FTempAssetHandler(nullptr, InDragStartData));

		CreateTempAssetData();

	} else {

		DraggingTempAssetHandler = MakeShareable(new FTempAssetHandler(nullptr,InDragStartData));

		if (!FAssetUtilities::TryToGetImportedExactProductData(InDragStartData,ProductFolderName, DraggedAssetData)) {

			CreateTempAssetData();
		}
	}

	UActorFactory *FoundFactory = FAssetActorFactoryHelper::GetActorFactoryForAsset(DraggedAssetData);
	const TSharedRef<FAssetDragDrop> DragDropOperation = FAssetDragDrop::New(DraggedAssetData, FoundFactory, InDragStartData.HTMLContent, InDragStartData.Hash);
	DragDropOperation->OnRocketAssetDropped.BindUObject(this, &UFileDownloadHandler::OnRocketAssetDropped);

	bIsDragging = true;

	CreateOnActorsDroppedEvent();

	FAssetDragDrop::SwitchDragDropOp(DragDropOperation);

	return true;
}

void UFileDownloadHandler::OnPluginsActivationRespondedForProduct(const TArray<FPluginViewItemInfo> &InPluginsInfo, const FName &InProductName, bool InApproved) const {

	FRocketDependencyUIManager::ResetWindowData();
	
	if(InApproved) {
		for(const FPluginViewItemInfo& CurrentPluginsInfo: InPluginsInfo) {

			FRocketDependencyManager::TryEnablePlugin(CurrentPluginsInfo.PluginName.ToString());
		}
		FRocketDependencyManager::SuggestRestart();
	}
}

bool UFileDownloadHandler::ImportEnvironment(const FString &InEnvironmentName) const {
    
	FString EnvironmentPath = FPaths::Combine(*FPathHelpers::GetRocketDepotPath(), TEXT("products"), InEnvironmentName);
	TArray<FString> EnabledPlugins = FRocketDependencyManager::GetEnabledPlugins(EnvironmentPath);

	TArray<FString> MustActivateEnginePluginList;

	for (const FString& PluginName : EnabledPlugins)
	{
		if (!FRocketDependencyManager::IsPluginEnabled(PluginName) && FRocketDependencyManager::CheckIfEnginePlugin(PluginName))
		{
			MustActivateEnginePluginList.Add(PluginName);
		}
	}

	if (MustActivateEnginePluginList.IsEmpty()) {
		ProceedImportEnvironment(false, InEnvironmentName);
	} else {
		
		TArray<FPluginViewItemInfo> PluginViewInfo = FRocketDependencyManager::CreatePluginViewItemInfoArray(MustActivateEnginePluginList);
		FText Message = FText::FromName(TEXT("The following plugins need to be enabled to import this project. Would you like to activate them now?"));
		FRocketDependencyUIManager::SpawnPluginEnableWindow(InEnvironmentName,Message, PluginViewInfo, FOnPluginsActivationResponseSignature::CreateUObject(this, &UFileDownloadHandler::OnPluginsActivationRespondedForEnvironment));
	}
	
	return true;
}

void UFileDownloadHandler::OnPluginsActivationRespondedForEnvironment(const TArray<struct FPluginViewItemInfo>& InPluginsInfo, const FName& InEnvironmentName, bool InApproved) const {
	
	FRocketDependencyUIManager::ResetWindowData();
	
	if(InApproved) {
		for(const FPluginViewItemInfo& CurrentPluginsInfo: InPluginsInfo) {

			FRocketDependencyManager::TryEnablePlugin(CurrentPluginsInfo.PluginName.ToString());
		}
		
		ProceedImportEnvironment(true, InEnvironmentName.ToString());
	}
}

void  UFileDownloadHandler::ProceedImportEnvironment(bool bIsSuggestingRestart, const FString &InEnvironmentName) const {
	
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FString FullFolderPath = FPaths::Combine(*FPathHelpers::GetRocketDepotPath(), TEXT("products"), InEnvironmentName, TEXT("Content"));

	if (!PlatformFile.DirectoryExists(*FullFolderPath)) {
		UE_LOG(LogRocket, Error, TEXT("Directory does not exist for importing the environment: %s"), *FullFolderPath);
		return;
	}

	TArray<FString> SourceFiles;
	PlatformFile.FindFilesRecursively(SourceFiles, *FullFolderPath, nullptr);

	SourceFiles.RemoveAll([](const FString& FilePath) {
		return !(FilePath.EndsWith(TEXT(".uasset")) || FilePath.EndsWith(TEXT(".umap")));
	});

	if (SourceFiles.Num() == 0) {
		UE_LOG(LogRocket, Warning, TEXT("No valid asset files found for importing in directory: %s"), *FullFolderPath);
		return;
	}

	Async(EAsyncExecution::Thread, [this,bIsSuggestingRestart, SourceFiles, FullFolderPath]() {
		IPlatformFile &PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

		for (const FString& SourceFile : SourceFiles) {

			FString RelativeFilePath = SourceFile.Mid(FullFolderPath.Len() + 1);
			FString DestinationPath = FPaths::Combine(FPaths::ProjectContentDir(), RelativeFilePath);

			if (PlatformFile.FileExists(*DestinationPath)) {
				continue;
			}

			if (!PlatformFile.DirectoryExists(*FPaths::GetPath(DestinationPath))) {
				PlatformFile.CreateDirectoryTree(*FPaths::GetPath(DestinationPath));
			}

			if (!PlatformFile.CopyFile(*DestinationPath, *SourceFile)) {
				UE_LOG(LogRocket, Error, TEXT("Error occurred while copying asset: %s"), *DestinationPath);
			}
		}

		AsyncTask(ENamedThreads::GameThread, [this, bIsSuggestingRestart, SourceFiles, FullFolderPath]() {

			TArray<FString> EnvFolderNames;

			for (const FString& SourceFile : SourceFiles) {
				FString RelativeFilePath = SourceFile.Mid(FullFolderPath.Len() + 1);
				int32 SlashIndex;

				if (RelativeFilePath.FindChar(TEXT('/'), SlashIndex)) {
					FString FirstFolderName = RelativeFilePath.Left(SlashIndex);

					if (!EnvFolderNames.Contains(FirstFolderName)) {
						EnvFolderNames.Add(FirstFolderName);
					}
				}
			}

			if (EnvFolderNames.Num() > 0) {
				const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
				TArray<FString> PathsToScan;

				for (const FString& FolderName : EnvFolderNames) {
					PathsToScan.Add(FPaths::Combine(FPaths::ProjectContentDir(), FolderName));
				}

				AssetRegistryModule.Get().ScanPathsSynchronous(PathsToScan, true);

				EnvFolderNames.RemoveAll([](const FString& FolderName) {
					return FolderName.Equals(TEXT("Collections")) || FolderName.Equals(TEXT("Developers")) || FolderName.Equals(TEXT("StarterContent"));
				});

				if (!EnvFolderNames.IsEmpty()) {
					const FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
					TArray<FString> FoldersToSync;
					FoldersToSync.Add(FPaths::Combine(TEXT("/Game"), EnvFolderNames[0]));
					ContentBrowserModule.Get().SyncBrowserToFolders(FoldersToSync);
				}
			}

			if(bIsSuggestingRestart) {
				FRocketDependencyManager::SuggestRestart();
			}
		});
	});
}



bool UFileDownloadHandler::TryToImportProductToProject(const FString &InAssetType,const FString &InObjectType, const FString &InProductName, const FString &InHash,bool IsInteractive, bool OpenInContentDrawer) {

	TArray<FString> FoundDependencies = FRocketDependencyManager::FindPluginDependencies(InAssetType,InObjectType,IsInteractive);

	if(!FoundDependencies.IsEmpty()) {
		
		TArray<FString> MustActivateEnginePluginList;

		for (const FString& PluginName : FoundDependencies)
		{
			if (!FRocketDependencyManager::IsPluginEnabled(PluginName) && FRocketDependencyManager::CheckIfEnginePlugin(PluginName))
			{
				MustActivateEnginePluginList.Add(PluginName);
			}
		}
		if(!MustActivateEnginePluginList.IsEmpty()) {
			TArray<FPluginViewItemInfo> PluginViewInfo = FRocketDependencyManager::CreatePluginViewItemInfoArray(MustActivateEnginePluginList);
			FText Message = FText::FromName(TEXT("The following plugins need to be enabled to use this asset. Would you like to activate them now?"));
			FRocketDependencyUIManager::SpawnPluginEnableWindow(InProductName, Message, PluginViewInfo, FOnPluginsActivationResponseSignature::CreateUObject(this, &UFileDownloadHandler::OnPluginsActivationRespondedForProduct));
			return false;
		}
	}
	
	static const TArray ImportableProducts = {
		ERocketProductType::Asset, 
		ERocketProductType::Vfx, 
		ERocketProductType::Material, 
		ERocketProductType::Sound, 
		ERocketProductType::Decal
	};
	
	ERocketProductType Type = FAssetUtilities::GetAssetTypeFromString(InAssetType);
	if (!ImportableProducts.Contains(Type)) {
		return false;
	}
	
	FString LocalProductFolderName = InProductName + '_' + InHash + '/' + InProductName;
	FThreadSafeCounter TotalTasks;
	FThreadSafeCounter CompletedTasks;

	Async(EAsyncExecution::Thread, [this, LocalProductFolderName, InProductName, InHash, OpenInContentDrawer, Type, &TotalTasks, &CompletedTasks]() {
		IPlatformFile &PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		FString FullFolderPath;

		if (Type == ERocketProductType::Sound) {
			FullFolderPath = FPaths::Combine(*FPathHelpers::GetRocketDepotPath(), TEXT("products"), LocalProductFolderName);
		} else {
			FullFolderPath = FPaths::Combine(*FPathHelpers::GetRocketDepotPath(), TEXT("products"), LocalProductFolderName, TEXT("Unreal"), TEXT("Rocket"));
		}

		if (!PlatformFile.DirectoryExists(*FullFolderPath)) {
			UE_LOG(LogRocket, Warning, TEXT("Directory does not exist for importing: %s"), *FullFolderPath);
			return;
		}
		
		TArray<FString> SourceFiles;

		if (Type == ERocketProductType::Sound) {
			TArray<FString> WAVFiles;
			TArray<FString> MP3Files;
			PlatformFile.FindFilesRecursively(WAVFiles, *FullFolderPath, TEXT("wav"));
			PlatformFile.FindFilesRecursively(MP3Files, *FullFolderPath, TEXT("mp3"));

			SourceFiles.Append(WAVFiles);
			SourceFiles.Append(MP3Files);
		} else {
			PlatformFile.FindFilesRecursively(SourceFiles, *FullFolderPath, TEXT("uasset"));
		}
		if (SourceFiles.Num() > 0) {
			if (IsProductUAsset(Type)) {
				for (const FString &SourceFile : SourceFiles) {
					FString FileName = SourceFile;

					FileName.ReplaceInline(*FullFolderPath, TEXT(""));

					while (FileName.StartsWith(TEXT("/")) || FileName.StartsWith(TEXT("\\"))) {
						FileName = FileName.Mid(1);
					}

					FString DestinationPath = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Rocket"), FileName);
					
					if (PlatformFile.FileExists(*DestinationPath)) {
						continue;
					}

					if (!PlatformFile.DirectoryExists(*FPaths::GetPath(DestinationPath))) {
						PlatformFile.CreateDirectoryTree(*FPaths::GetPath(DestinationPath));
					}

					if (!PlatformFile.CopyFile(*DestinationPath, *SourceFile)) {
						UE_LOG(LogRocket, Error, TEXT("Error occurred while copying asset: %s"), *DestinationPath);
					}
				}
			}
			AsyncTask(ENamedThreads::GameThread, [this, InProductName, InHash, OpenInContentDrawer, Type, SourceFiles, FullFolderPath]() {
				if (Type == ERocketProductType::Sound) {
					FAssetUtilities::CreateSoundWaveAsset(FullFolderPath, InProductName);

					ImportCompleted(InProductName, InHash, Type);

					if (OpenInContentDrawer) {
						const FString FolderToFocus = FPaths::Combine(TEXT("/Game/Rocket"), InProductName);
						const FContentBrowserModule &ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
						TArray<FString> Folders;
						Folders.Add(FolderToFocus);
						ContentBrowserModule.Get().SyncBrowserToFolders(Folders);
					}
				} else {
					FString ProductFolderNameToScan = FPathHelpers::FindMainFolderNameForProduct(InProductName, InHash, Type);

					const FAssetRegistryModule &AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
					TArray<FString> PathsToScan;
					PathsToScan.Add(FPaths::Combine(FPaths::ProjectDir(), TEXT("Content"), TEXT("Rocket"), TEXT("Masters")));
					PathsToScan.Add(FPaths::Combine(FPaths::ProjectDir(), TEXT("Content"), TEXT("Rocket"), ProductFolderNameToScan));
					AssetRegistryModule.Get().ScanPathsSynchronous(PathsToScan, true);

					ImportCompleted(InProductName, InHash, Type);

					if (OpenInContentDrawer) {
						const FString FolderToFocus = FPaths::Combine(TEXT("/Game/Rocket"), ProductFolderNameToScan);
						const FContentBrowserModule &ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
						TArray<FString> Folders;
						Folders.Add(FolderToFocus);
						ContentBrowserModule.Get().SyncBrowserToFolders(Folders);
					}
				}
			});
		}
	});

	return true;
}

FString UFileDownloadHandler::CreateNewEnvironment(const FString &InEnvironmentName) {
	const FString DefaultProjectPath = FPaths::Combine(FPathHelpers::GetHomeDirectory(), TEXT("Documents/Unreal Projects"));
	const FString ProjectFolderName = InEnvironmentName + TEXT(".zip");

	FString TargetFolder;

	if (IDesktopPlatform *DesktopPlatform = FDesktopPlatformModule::Get()) {
		const void *ParentWindowHandle = nullptr;
		if (GEngine && GEngine->GameViewport) {
			ParentWindowHandle = GEngine->GameViewport->GetWindow()->GetNativeWindow()->GetOSWindowHandle();
		}
		const FString Title = TEXT("Enter folder name for the environment");
		const FString DefaultFile = InEnvironmentName;

		TArray<FString> OutFilenames;
		if (DesktopPlatform->SaveFileDialog(ParentWindowHandle, Title, DefaultProjectPath, DefaultFile, TEXT("All Files (*.*)|*.*"), EFileDialogFlags::None, OutFilenames)) {
			if (OutFilenames.Num() > 0) {
				const FString SelectedFolder = OutFilenames[0];
				FString EnteredFolderName = FPaths::GetBaseFilename(SelectedFolder);
				TargetFolder = FPaths::Combine(FPaths::GetPath(SelectedFolder), EnteredFolderName);
			} else {
				UE_LOG(LogRocket, Warning, TEXT("No file name entered"));
				return TEXT("");
			}
		} else {
			return TEXT("");
		}
	}

	if (FPaths::DirectoryExists(TargetFolder)) {
		UE_LOG(LogRocket, Log, TEXT("Project folder already exists: %s"), *TargetFolder);
		UE_LOG(LogRocket, Warning, TEXT("The folder is not empty. Extraction aborted."));
		return TEXT("");
	}
	
	if (IFileManager::Get().MakeDirectory(*TargetFolder, true)) {
		const FString ZipFilePath = FPaths::Combine(FPathHelpers::GetRocketDepotPath(), TEXT("compressed/") + ProjectFolderName);
		FArchiver::UnzipFile(ZipFilePath, TargetFolder, true);
		TArray<FString> Files;
		IFileManager::Get().FindFilesRecursive(Files, *TargetFolder, TEXT("*.uproject"), true, false, false);

		if (Files.Num() > 0) {
			const FString ProjectFilePath = Files[0];
			if (FPaths::FileExists(ProjectFilePath)) {
				const FString AbsolutePath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*ProjectFilePath);
				if (FPlatformProcess::LaunchFileInDefaultExternalApplication(*AbsolutePath, nullptr, ELaunchVerb::Open)) {
					return AbsolutePath;
				}
				UE_LOG(LogRocket, Error, TEXT("Failed to open .uproject file: %s"), *ProjectFilePath);
				return TEXT("");
			}
			UE_LOG(LogRocket, Error, TEXT(".uproject file not found after extraction: %s"), *ProjectFilePath);
			return TEXT("");
		}
		UE_LOG(LogRocket, Error, TEXT("No .uproject file found in the extracted folder: %s"), *TargetFolder);
		return TEXT("");
	}
	UE_LOG(LogRocket, Error, TEXT("Failed to create directory: %s"), *TargetFolder);
	return TEXT("");
}



void UFileDownloadHandler::CreateTempAssetData() {
	if (!IsValid(Instance)) {
		return;
	}

	Instance->DraggedAssetData = LoadObject<UStaticMesh>(nullptr, *RocketAssetData::LightMapToolWindowPath.ToString());
}

bool UFileDownloadHandler::IsProductUAsset(const ERocketProductType& Type) {
	static const TArray UAssetTypes = {
		ERocketProductType::Asset,
		ERocketProductType::Vfx,
		ERocketProductType::Material,
		ERocketProductType::Decal
	};

	return UAssetTypes.Contains(Type);
}


//Only dropped assets will be dealt with here
void UFileDownloadHandler::OnAssetProcessingCompleted(const FString &InHash) {

	TSharedPtr<FTempAssetHandler> *FoundTempAssetHandler = TempAssetData.Find(InHash);

	if (FoundTempAssetHandler && FoundTempAssetHandler->IsValid() && FoundTempAssetHandler->Get()) {

		ReplaceTempAssetsWithProcessedAsset(FoundTempAssetHandler->Get());

		TempAssetData.Remove(InHash);
	}
}

void UFileDownloadHandler::RemoveCancelledTempAssetsWithHash(const FString &InHash) {
	TArray<FString> Keys;
	Instance->TempAssetData.GetKeys(Keys);
	//iterate from the end to the end
	for (int32 i = Keys.Num() - 1; i >= 0; i--) {

		auto CurrentKey = Keys[i];

		if(InHash.EndsWith(CurrentKey)) {

			 TSharedPtr<FTempAssetHandler> FoundTempAssetHandler = Instance->TempAssetData[CurrentKey];
			if(FoundTempAssetHandler.IsValid() && FoundTempAssetHandler.Get()) {
				TArray<TWeakObjectPtr<AActor>> TempActors = FoundTempAssetHandler->GetTempActors();
				for (TWeakObjectPtr<AActor> TempActor : TempActors) {
					if (TempActor.IsValid()) {
						TempActor->Destroy();
					}
				}

				// Remove the entry from TempAssetData
				Instance->TempAssetData.Remove(CurrentKey);
				return;
			}
		}
	}
}


void UFileDownloadHandler::ReplaceTempAssetsWithProcessedAsset(FTempAssetHandler *InTempAssetHandler) {
	const FString ImportedProductFolderName = FPathHelpers::FindMainFolderNameForProduct(InTempAssetHandler->GetProductName(), InTempAssetHandler->GetHash(), FAssetUtilities::GetAssetTypeFromString(InTempAssetHandler->GetAssetType()));

	if (InTempAssetHandler->GetTempActors().IsEmpty()) {
		return;
	}
	
	ERocketProductType Type = FAssetUtilities::GetAssetTypeFromString(InTempAssetHandler->GetAssetType());
	
	if (Type == ERocketProductType::Asset || Type == ERocketProductType::Vfx) {
		FAssetData LocalAssetData;

		FAssetUtilities::TryToGetImportedExactProductData(InTempAssetHandler->GetProductData(), ImportedProductFolderName,LocalAssetData);

		for (auto CurrentTempActor : InTempAssetHandler->GetTempActors()) {
			if (!CurrentTempActor.IsValid()) {
				continue;
			}

			if (!LocalAssetData.IsValid() || !IsValid(LocalAssetData.GetAsset())) {
				UE_LOG(LogRocket, Error, TEXT("Failed to find imported asset data for asset or vfx: %s"), *InTempAssetHandler->GetProductName());
				continue;
			}

			UWorld *CurrentWorld = CurrentTempActor->GetWorld();
			if (!IsValid(CurrentWorld)) {
				continue;
			}

			if (IsValid(CurrentWorld)) {
				TObjectPtr<UBlueprint> FoundBlueprint = Cast<UBlueprint>(LocalAssetData.GetAsset());
				if (IsValid(FoundBlueprint)) {
					UClass *BlueprintClass = FoundBlueprint->GeneratedClass;
					if (IsValid(BlueprintClass)) {
						AActor *SpawnedActor = CurrentWorld->SpawnActor<AActor>(BlueprintClass, CurrentTempActor->GetTransform());
						FName NewActorName = MakeUniqueObjectName(this, BlueprintClass, FName(*FoundBlueprint->GetName()));
						SpawnedActor->SetActorLabel(NewActorName.ToString());

						if (GEditor && CurrentTempActor.IsValid() && GEditor->GetSelectedActors()->IsSelected(CurrentTempActor.Get())) {
							GEditor->SelectNone(false, true, false);
							GEditor->SelectActor(SpawnedActor, true, true, false, true);
						}

						if (CurrentTempActor.IsValid()) {
							CurrentTempActor->Destroy();
						}
						continue;
					}
				}

				TObjectPtr<UStaticMesh> FoundStaticMesh = Cast<UStaticMesh>(LocalAssetData.GetAsset());
				if (IsValid(FoundStaticMesh)) {
					if (AStaticMeshActor *StaticMeshActor = CurrentWorld->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), CurrentTempActor->GetTransform())) {
						StaticMeshActor->GetStaticMeshComponent()->SetStaticMesh(FoundStaticMesh);
						FName NewActorName = MakeUniqueObjectName(this, StaticMeshActor->GetClass(), FName(*FoundStaticMesh->GetName()));
						StaticMeshActor->SetActorLabel(NewActorName.ToString());

						if (GEditor && CurrentTempActor.IsValid() && GEditor->GetSelectedActors()->IsSelected(CurrentTempActor.Get())) {
							GEditor->SelectNone(false, true, false);
							GEditor->SelectActor(StaticMeshActor, true, true, false, true);
						}

						if (CurrentTempActor.IsValid()) {
							CurrentTempActor->Destroy();
						}

						continue;
					}
				}

				TObjectPtr<UNiagaraSystem> FoundNiagaraSystem = Cast<UNiagaraSystem>(LocalAssetData.GetAsset());
				if (IsValid(FoundNiagaraSystem)) {
					if (ANiagaraActor *NiagaraActor = CurrentWorld->SpawnActor<ANiagaraActor>(ANiagaraActor::StaticClass(), CurrentTempActor->GetTransform())) {
						FName NewActorName = MakeUniqueObjectName(this, NiagaraActor->GetClass(), FName(*FoundNiagaraSystem->GetName()));
						NiagaraActor->GetNiagaraComponent()->SetAsset(FoundNiagaraSystem);
						NiagaraActor->SetActorLabel(NewActorName.ToString());

						if (GEditor && CurrentTempActor.IsValid() && GEditor->GetSelectedActors()->IsSelected(CurrentTempActor.Get())) {
							GEditor->SelectNone(false, true, false);
							GEditor->SelectActor(NiagaraActor, true, true, false, true);
						}

						if (CurrentTempActor.IsValid()) {
							CurrentTempActor->Destroy();
						}
					}
				}
			}
		}
	} else if (Type == ERocketProductType::Material) {
		FAssetData LocalAssetData;
		FAssetUtilities::TryToGetImportedExactProductMaterialData(InTempAssetHandler->GetProductName(), FAssetUtilities::GetResolutionFromLOD(InTempAssetHandler->GetLOD()), LocalAssetData);
		for (auto CurrentTempActor : InTempAssetHandler->GetTempActors()) {
			if (CurrentTempActor.IsValid()) {

				FAssetUtilities::ApplyMaterialToComponent(CurrentTempActor.Get(), InTempAssetHandler->GetProductName(), InTempAssetHandler->GetLOD());
			}
		}

	} else if (Type == ERocketProductType::Sound) {
		FString SoundDirectory = FString::Printf(TEXT("/Game/Rocket/%s"), *InTempAssetHandler->GetProductName());
		TArray<FAssetData> AssetDataArray;
		const FAssetRegistryModule &AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		IAssetRegistry &AssetRegistry = AssetRegistryModule.Get();

		FARFilter Filter;
		Filter.PackagePaths.Add(*SoundDirectory);

		AssetRegistry.GetAssets(Filter, AssetDataArray);

		if (AssetDataArray.Num() > 0) {
			FUIHelpers::ShowSoundSelectionWindow(this, AssetDataArray, InTempAssetHandler->GetProductName(), InTempAssetHandler->GetHash());
		} else {
			UE_LOG(LogRocket, Error, TEXT("No sound assets found in directory: %s"), *SoundDirectory);
		}
	}
}



void UFileDownloadHandler::CreateOnActorsDroppedEvent() {
	FEditorDelegates::OnNewActorsDropped.AddUObject(this, &UFileDownloadHandler::OnActorsDropped);
}

void UFileDownloadHandler::DestroyOnActorsDroppedEvent() const {
	FEditorDelegates::OnNewActorsDropped.RemoveAll(this);
}

void UFileDownloadHandler::OnActorsDropped(const TArray<UObject *> &InObjects, const TArray<AActor *> &InActors) {

	if (!IsValid(Instance)) {
		UE_LOG(LogRocket, Error, TEXT("Instance is not valid in SetupOnActorsDroppedEvent"));
		return;
	}
	if (!DraggingTempAssetHandler.IsValid()) {
		UE_LOG(LogRocket, Error, TEXT("DraggedAssetData is not valid"));
		return;
	}

	ERocketProductType DraggingType = FAssetUtilities::GetAssetTypeFromString(DraggingTempAssetHandler->GetAssetType());

	if (bIsDragging) {
		for (int32 i = 0; i < InActors.Num(); i++) {
			AActor *DroppedActor = InActors[i];
			if (!IsValid(DroppedActor)) {
				UE_LOG(LogRocket, Warning, TEXT("DroppedActor is not valid"));
				continue;
			}

			if (DraggingType == ERocketProductType::Asset || DraggingType == ERocketProductType::Vfx) {
				if (DroppedActor->GetClass()->GetName().Equals(TEXT("StaticMeshActor"))) {
					if (const auto SphereSMActor = Cast<AStaticMeshActor>(DroppedActor)) {
						if (SphereSMActor && SphereSMActor->GetStaticMeshComponent() && SphereSMActor->GetStaticMeshComponent()->GetStaticMesh()) {
							if (SphereSMActor->GetStaticMeshComponent()->GetStaticMesh().GetPath().Contains(FPaths::GetBaseFilename(FSoftObjectPath(RocketAssetData::LightMapToolWindowPath).ToString()))) {
								if (DraggingTempAssetHandler.IsValid()) {

									if (auto FoundExistingTempData = TempAssetData.Find(DraggingTempAssetHandler->GetHash())) {
										FoundExistingTempData->Get()->AddNewTempActor(DroppedActor);
									} else {
										DraggingTempAssetHandler->AddNewTempActor(DroppedActor);
										DraggingTempAssetHandler->SetCallback(FOnAssetProcessingCompleted::CreateUObject(this, &UFileDownloadHandler::OnAssetProcessingCompleted));
										TempAssetData.Add(DraggingTempAssetHandler->GetHash(), DraggingTempAssetHandler);
									}

									CleanDragData();
								}
								UE_LOG(LogRocket, Warning, TEXT("DraggingTempAssetHandler is not valid"));
							} else {
								UE_LOG(LogRocket, Warning, TEXT("StaticMesh does not contain 'Sphere' in its path"));
							}
						} else {
							UE_LOG(LogRocket, Warning, TEXT("SphereSMActor or its StaticMeshComponent is not valid"));
						}
					} else {
						UE_LOG(LogRocket, Warning, TEXT("DroppedActor is not a StaticMeshActor"));
					}
				}
			} else if (DraggingType == ERocketProductType::Sound) {
				if (DroppedActor->GetClass()->GetName().Equals(TEXT("StaticMeshActor"))) {
					if (const auto SphereSMActor = Cast<AStaticMeshActor>(DroppedActor)) {
						if (SphereSMActor && SphereSMActor->GetStaticMeshComponent() && SphereSMActor->GetStaticMeshComponent()->GetStaticMesh()) {
							if (SphereSMActor->GetStaticMeshComponent()->GetStaticMesh().GetPath().Contains(FPaths::GetBaseFilename(FSoftObjectPath(RocketAssetData::LightMapToolWindowPath).ToString()))) {
								if (DraggingTempAssetHandler.IsValid()) {

									if (auto FoundExistingTempData = TempAssetData.Find(DraggingTempAssetHandler->GetHash())) {
										FoundExistingTempData->Get()->AddNewTempActor(DroppedActor);
									} else {
										DraggingTempAssetHandler->AddNewTempActor(DroppedActor);
										DraggingTempAssetHandler->SetCallback(FOnAssetProcessingCompleted::CreateUObject(this, &UFileDownloadHandler::OnAssetProcessingCompleted));
										TempAssetData.Add(DraggingTempAssetHandler->GetHash(), DraggingTempAssetHandler);
									}

									const FString ProductFolderName = FPathHelpers::FindMainFolderNameForProduct(DraggingTempAssetHandler->GetProductName(), DraggingTempAssetHandler->GetHash(), FAssetUtilities::GetAssetTypeFromString(DraggingTempAssetHandler->GetAssetType()));
									FAssetData LocalAssetData;
									FAssetUtilities::TryToGetImportedExactProductData(DraggingTempAssetHandler->GetProductData(), ProductFolderName, LocalAssetData);

									//Asset is imported, so show the pop-up
									if (LocalAssetData.IsValid()) {

										OnAssetProcessingCompleted(DraggingTempAssetHandler->GetHash());
									}

									CleanDragData();
								}
								UE_LOG(LogRocket, Warning, TEXT("DraggingTempAssetHandler is not valid"));
							} else {
								UE_LOG(LogRocket, Warning, TEXT("StaticMesh does not contain 'Sphere' in its path"));
							}
						} else {
							UE_LOG(LogRocket, Warning, TEXT("SphereSMActor or its StaticMeshComponent is not valid"));
						}
					} else {
						UE_LOG(LogRocket, Warning, TEXT("DroppedActor is not a StaticMeshActor"));
					}
				}
			}
		}
	}
}

void UFileDownloadHandler::OnRocketAssetDropped(const FPointerEvent &MouseEvent) {

	if (bIsDragging && DraggingTempAssetHandler.IsValid() && FAssetUtilities::GetAssetTypeFromString(DraggingTempAssetHandler->GetAssetType()) == ERocketProductType::Material) {

		AActor *LocalHitActor = FRocketTraceHelper::GetActorUnderMousePosition(MouseEvent.GetScreenSpacePosition());

		if (LocalHitActor && LocalHitActor->IsA(AStaticMeshActor::StaticClass())) {

			//It means it did not download yet
			if (auto FoundExistingTempData = TempAssetData.Find(DraggingTempAssetHandler->GetHash())) {

				FoundExistingTempData->Get()->AddNewTempActor(LocalHitActor);

			} else {

				DraggingTempAssetHandler->AddNewTempActor(LocalHitActor);
				DraggingTempAssetHandler->SetCallback(FOnAssetProcessingCompleted::CreateUObject(this, &UFileDownloadHandler::OnAssetProcessingCompleted));
				TempAssetData.Add(DraggingTempAssetHandler->GetHash(), DraggingTempAssetHandler);

				FAssetUtilities::TryToGetImportedExactProductMaterialData(DraggingTempAssetHandler->GetProductName(), FAssetUtilities::GetResolutionFromLOD(DraggingTempAssetHandler->GetLOD()), DraggedAssetData);
				if (DraggedAssetData.IsValid() && IsValid(DraggedAssetData.GetAsset()) && DraggedAssetData.GetAsset()->IsA(UMaterialInterface::StaticClass())) {

					OnAssetProcessingCompleted(DraggingTempAssetHandler->GetHash());
				}
			}
		}
	}
}

void UFileDownloadHandler::CleanDragData() {

	DestroyOnActorsDroppedEvent();

	bIsDragging = false;

	DraggingTempAssetHandler = nullptr;

	DraggedAssetData = FAssetData();

}

void UFileDownloadHandler::ImportCompleted(const FString &InProductName, const FString &Hash, const ERocketProductType &Type) {

	//materials will be handled on drop events
	if (!IsValid(Instance) || Type == ERocketProductType::Material) {
		return;
	}

	const FString ImportedProductFolderName = FPathHelpers::FindMainFolderNameForProduct(InProductName, Hash, Type);

	if (bIsDragging && DraggingTempAssetHandler.IsValid() && Type != ERocketProductType::Sound && DraggingTempAssetHandler->GetHash().Equals(Hash)) {
		
		const FString DraggingProductFolderNameWithHash = FPathHelpers::CombineProductNameWithHash(DraggingTempAssetHandler->GetProductName(), Hash, Type);
		
		if (DraggingTempAssetHandler->GetTargetAssetName().IsEmpty() && DraggingProductFolderNameWithHash.IsEmpty()) {
			return;
		}
		DraggedAssetData = FAssetUtilities::TryToFindImportedDraggedAssetData(DraggingTempAssetHandler);
		
		if (DraggedAssetData.IsValid()) {
			
			UActorFactory *FoundFactory = FAssetActorFactoryHelper::GetActorFactoryForAsset(DraggedAssetData);
			const TSharedRef<FAssetDragDropOp> DragDropOperation = FAssetDragDrop::New(DraggedAssetData, FoundFactory, DraggingTempAssetHandler->GetHTMLContent(), DraggingTempAssetHandler->GetHash());
			FAssetDragDrop::SwitchDragDropOp(DragDropOperation);
		}
	}

	bool bIsStartReplacement = true;
	if (Type == ERocketProductType::Sound && bIsDragging && DraggingTempAssetHandler.IsValid() && DraggingTempAssetHandler->GetHash().Equals(Hash)) {
		bIsStartReplacement = false;
	}

	if (bIsStartReplacement) {
		TSharedPtr<FTempAssetHandler> *FoundTempAssetHandler = TempAssetData.Find(Hash);
		if (FoundTempAssetHandler && FoundTempAssetHandler->IsValid() && FoundTempAssetHandler->Get()) {
			FoundTempAssetHandler->Get()->CallCallback();
		}
	}
}
