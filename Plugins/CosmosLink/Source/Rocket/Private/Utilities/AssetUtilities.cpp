// Copyright 2024 Leartes Studios. All Rights Reserved.

#include "AssetUtilities.h"
#include "AssetToolsModule.h"
#include "Editor.h"
#include "GeneralUtilities.h"
#include "NiagaraSystem.h"
#include "PathHelpers.h"
#include "RocketModule.h"
#include "ScopedTransaction.h"
#include "TempAssetHandler.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/AssetData.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Blueprint.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Factories/SoundFactory.h"
#include "HAL/PlatformFileManager.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Misc/FeedbackContext.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Sound/SoundWave.h"
#include "UObject/SavePackage.h"

class FTempAssetHandler;

FAssetData FAssetUtilities::TryToFindImportedDraggedAssetData(const TSharedPtr<FTempAssetHandler> &InDraggedAssetHandler) {
	if (!InDraggedAssetHandler.IsValid()) {
		return FAssetData();
	}

	const FString ProductFolderName = FPathHelpers::FindMainFolderNameForProduct(InDraggedAssetHandler->GetProductName(), InDraggedAssetHandler->GetHash(), GetAssetTypeFromString(InDraggedAssetHandler->GetAssetType()));

	if (ProductFolderName.IsEmpty()) {
		return FAssetData();
	}

	FAssetData LocalAssetData;
	if (TryToGetImportedExactProductData(InDraggedAssetHandler->GetProductData(), ProductFolderName,LocalAssetData)) {
		return LocalAssetData;
	}

	return FAssetData();
}

bool FAssetUtilities::TryToGetImportedExactProductData(const FRocketProductData &InProductData, const FString &InProductFolderName, FAssetData &OutAssetData) {
	
	OutAssetData = FAssetData();

	ERocketProductType Type = GetAssetTypeFromString(InProductData.AssetType);

	const FString ProductDirectory = FPaths::Combine(TEXT("/Game"), TEXT("Rocket"), *InProductFolderName);

	if (!FPathHelpers::DoesAssetDirectoryExist(ProductDirectory)) {
		return false;
	}

	auto IsAssetValid = [](const FAssetData &AssetData) -> bool {
		return IsValid(AssetData.GetAsset());
	};
	
#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1)
	auto GetAssetDataForType = [&](const FString &Directory, const FString &TargetAssetFileName, const FString &LevelOfDetail, const FString &InProductName, const FString &Hash, const ERocketProductType &InProductType, const TArray<FTopLevelAssetPath> &AssetClassPaths, const TArray<FString> &InFilePrefixes, FAssetData &OutData, bool bIsRecursivePaths) -> bool {
		FARFilter TargetFilter;
		TargetFilter.PackagePaths.Add(FName(*Directory));
		TargetFilter.bRecursivePaths = bIsRecursivePaths;
		for (const auto &ClassPath : AssetClassPaths) {

			TargetFilter.ClassPaths.Add(ClassPath);
		}
#else

	auto GetAssetDataForType = [&](const FString &Directory, const FString &TargetAssetFileName, const FString &LevelOfDetail, const FString &InProductName, const FString &Hash,const ERocketProductType &InProductType, const TArray<FName> &AssetClassNames, const TArray<FString> &InFilePrefixes, FAssetData &OutData, bool bIsRecursivePaths) -> bool {
		FARFilter TargetFilter;
		TargetFilter.PackagePaths.Add(FName(*Directory));
		TargetFilter.bRecursivePaths = bIsRecursivePaths;
		for (const auto &ClassName : AssetClassNames) {
			TargetFilter.ClassNames.Add(ClassName);
		}
#endif

		FString LevelOfDetailLocal = LevelOfDetail;
		if(Type == ERocketProductType::Decal || Type == ERocketProductType::Material) {
			LevelOfDetailLocal = FString::FromInt(GetResolutionFromLOD(LevelOfDetailLocal));
		}
		
		const FAssetRegistryModule &AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
		IAssetRegistry &AssetRegistry = AssetRegistryModule.Get();
		AssetRegistry.ScanPathsSynchronous({*Directory});

		TArray<FAssetData> FoundAssets;
		AssetRegistry.GetAssets(TargetFilter, FoundAssets);
		if (FoundAssets.IsEmpty()) {
			return false;
		}
		
		if (FoundAssets.Num() == 1) {
			OutData = FoundAssets[0];
			return true;
		}

		if(InProductData.bIsInteractive && GetObjectTypeFromString(InProductData.ObjectType) == ERocketObjectType::Vehicle) {
			for (const FAssetData &FoundAsset : FoundAssets) {
				TMap<FName, FString> Metadata = GetProductMetadata(FoundAsset);
				
				if(Metadata.IsEmpty()){continue;}
				for(TPair<FName, FString> CurrentMetadata : Metadata) {
					if(CurrentMetadata.Key.IsEqual(InteractiveMetadataKey)) {
						OutData = FoundAsset;
						return true;		
					}
				}
			}
		}
		
		if (!TargetAssetFileName.IsEmpty()) {
			for (const FAssetData &FoundAsset : FoundAssets) {
				if (IsAssetValid(FoundAsset)) {
					const FString GeneratedFileName = TargetAssetFileName + (LevelOfDetailLocal.IsEmpty() ? "" : TEXT("_") + LevelOfDetailLocal.ToUpper());
					if (FoundAsset.AssetName.ToString().Equals(GeneratedFileName, ESearchCase::IgnoreCase)) {
						OutData = FoundAsset;
						return true;
					}
				}
			}
		}

		for (const FString &CurrentPrefix : InFilePrefixes) {
			FString FoundFileName = FindMatchingAssetName(InProductName, Hash, LevelOfDetailLocal, CurrentPrefix, Type);
			if (!FoundFileName.IsEmpty()) {
				for (const FAssetData &FoundAsset : FoundAssets) {

					if (IsAssetValid(FoundAsset) && FoundAsset.AssetName.ToString().Equals(FoundFileName, ESearchCase::IgnoreCase)) {
						OutData = FoundAsset;
						return true;
					}
				}
			}
		}
		return false;
	};

	bool bIsRecursivePaths = true;

#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1)

	TArray<FTopLevelAssetPath> ClassPaths;
	TArray<FString> FilePrefixes;

	switch (Type)
	{
	    case ERocketProductType::Asset:
	        ClassPaths.Add(UStaticMesh::StaticClass()->GetClassPathName());
	        ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
	        FilePrefixes = { TEXT("BP"), TEXT("SM") };
	        break;

	    case ERocketProductType::Vfx:
	        ClassPaths.Add(UNiagaraSystem::StaticClass()->GetClassPathName());
	        ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
	        FilePrefixes = { TEXT("BP"), TEXT("NS"), TEXT("NS1") };
	        break;

	    case ERocketProductType::Material:
	        ClassPaths.Add(UMaterialInstance::StaticClass()->GetClassPathName());
	        ClassPaths.Add(UMaterialInstanceConstant::StaticClass()->GetClassPathName());
	        FilePrefixes = { TEXT("MI") };
	        break;

	    case ERocketProductType::Decal:
	        ClassPaths.Add(UMaterialInstance::StaticClass()->GetClassPathName());
	        ClassPaths.Add(UMaterialInstanceConstant::StaticClass()->GetClassPathName());
	        FilePrefixes = { TEXT("MI") };
	        break;

	    case ERocketProductType::Sound:
	        ClassPaths.Add(USoundWave::StaticClass()->GetClassPathName());
	        FilePrefixes = { TEXT("A") };
	        break;
	    default:
	        UE_LOG(LogRocket, Warning, TEXT("Product type is not defined correctly"));
	        return false;
	}

#else

	TArray<FName> ClassPaths;
	TArray<FString> FilePrefixes;

	switch (Type)
	{
	    case ERocketProductType::Asset:
	        ClassPaths.Add(UStaticMesh::StaticClass()->GetFName());
	        ClassPaths.Add(UBlueprint::StaticClass()->GetFName());
	        FilePrefixes = { TEXT("BP"), TEXT("SM") };
	        break;

	    case ERocketProductType::Vfx:
	        ClassPaths.Add(UNiagaraSystem::StaticClass()->GetFName());
	        ClassPaths.Add(UBlueprint::StaticClass()->GetFName());
	        FilePrefixes = { TEXT("BP"), TEXT("NS"), TEXT("NS1") };
	        break;

	    case ERocketProductType::Material:
	        ClassPaths.Add(UMaterialInstance::StaticClass()->GetFName());
	        ClassPaths.Add(UMaterialInstanceConstant::StaticClass()->GetFName());
	        FilePrefixes = { TEXT("MI") };
	        break;

	    case ERocketProductType::Decal:
	        ClassPaths.Add(UMaterialInstance::StaticClass()->GetFName());
	        ClassPaths.Add(UMaterialInstanceConstant::StaticClass()->GetFName());
	        ClassPaths.Add(UBlueprint::StaticClass()->GetFName());
	        FilePrefixes = { TEXT("MI") };
	        break;

	    case ERocketProductType::Sound:
	        ClassPaths.Add(USoundWave::StaticClass()->GetFName());
	        FilePrefixes = { TEXT("A") };
	        break;
	    default:
	        UE_LOG(LogRocket, Warning, TEXT("Product type is not defined correctly"));
	        return false;
	}

#endif
	return GetAssetDataForType(ProductDirectory, InProductData.TargetAssetName, InProductData.LOD, InProductData.ProductName, InProductData.Hash,Type, ClassPaths, FilePrefixes, OutAssetData,bIsRecursivePaths);
}

bool FAssetUtilities::TryToGetImportedExactProductMaterialData(const FString &ProductName, int32 Quality, FAssetData &OutAssetData) {

	OutAssetData = FAssetData();

	const FString QualitySuffix = FString::Printf(TEXT("_%d"), Quality);
	const FString MaterialPath = FString::Printf(TEXT("/Game/Rocket/%s"), *ProductName);

	const FAssetRegistryModule &AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry &AssetRegistry = AssetRegistryModule.Get();

	TArray<FString> PathsToScan;
	PathsToScan.Add(MaterialPath);
	AssetRegistry.ScanPathsSynchronous(PathsToScan, true);

	FARFilter TargetFilter;
	TargetFilter.PackagePaths.Add(FName(*MaterialPath));

	TArray<FAssetData> FoundAssets;
	AssetRegistry.GetAssets(TargetFilter, FoundAssets);

	if (FoundAssets.Num() == 0) {
		//UE_LOG(LogRocket, Warning, TEXT("No assets found in directory: %s"), *MaterialPath);
		return false;
	}
		
	for (const FAssetData &FoundAsset : FoundAssets) {
		if (FoundAsset.IsValid()) {
			const FString MaterialPrefix = FString::Printf(TEXT("MI_%s"), *ProductName);
			const FString FullMaterialName = MaterialPrefix + QualitySuffix;
			
			if (FoundAsset.AssetName.ToString().StartsWith(MaterialPrefix, ESearchCase::IgnoreCase) && FoundAsset.AssetName.ToString().EndsWith(QualitySuffix, ESearchCase::IgnoreCase)) {
				OutAssetData = FoundAsset;
				return true;
			}
		} else {
			UE_LOG(LogRocket, Warning, TEXT("Invalid asset found: %s"), *FoundAsset.AssetName.ToString());
		}
	}

	UE_LOG(LogRocket, Warning, TEXT("Material not found: %s with %s quality"), *MaterialPath, *QualitySuffix);
	return false;
}

FString FAssetUtilities::FindMatchingMaterialName(const FString &ProductName, int32 Quality) {
	return FString::Printf(TEXT("/Game/Rocket/%s/MI_%s%s"), *ProductName, *ProductName, *FString::Printf(TEXT("_%d"), Quality));
}

	
TArray<FString> FindFilesWithExtension(const FString& FolderPath, const FString& Extension)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	TArray<FString> FoundFiles;

	PlatformFile.IterateDirectoryRecursively(*FolderPath, [&FoundFiles](const TCHAR* FilePath, bool bIsDirectory)
	{
		if (!bIsDirectory) 
		{
			FString FileName(FilePath);
			FoundFiles.Add(FileName);
		}
		return true;
	});
		
	TArray<FString> FilesWithExtension;
	for (const FString& File : FoundFiles)
	{
		FString FileExtension = FPaths::GetExtension(File);
		if (FileExtension.Equals(Extension, ESearchCase::IgnoreCase)) 
		{
			FilesWithExtension.Add(File);
		}
	}

	if (FilesWithExtension.Num() == 0)
	{
		UE_LOG(LogRocket, Log, TEXT("No files with extension '.%s' found in folder: %s"), *Extension, *FolderPath);
	}

	return FilesWithExtension;
}
	
FString FAssetUtilities::FindMatchingAssetName(const FString &InProductName, const FString &Hash, const FString &InLOD, const FString &InAssetPrefix, const ERocketProductType &Type) {

	const FString ProductSourceFolderPath = Type == ERocketProductType::Sound ? FPathHelpers::AssetDirectory(InProductName, Hash, Type) : FPaths::Combine(FPathHelpers::AssetDirectory(InProductName, Hash, Type), FPathHelpers::FindMainFolderNameForProduct(InProductName, Hash, Type));

	TArray<FString> Files;
	if (Type == ERocketProductType::Sound) {
		Files.Append(FindFilesWithExtension(ProductSourceFolderPath,TEXT("waw")));
		Files.Append(FindFilesWithExtension(ProductSourceFolderPath,TEXT("mp3")));
	} else {
		//IFileManager::Get().FindFilesRecursive(Files, *ProductSourceFolderPath, TEXT(".uasset"),true, false);
		Files = FindFilesWithExtension(ProductSourceFolderPath,TEXT("uasset"));
	}

	const FString Prefix = FString::Printf(TEXT("%s_"), *InAssetPrefix);

	if (Files.IsEmpty()) {
		UE_LOG(LogRocket, Log, TEXT("No files found in directory: %s"), *ProductSourceFolderPath);
		return FString();
	}
	if (Files.Num() == 1) {
		return FPaths::GetBaseFilename(Files[0]);
	}
	for (const FString &FileName : Files) {
		FString BaseFileName = FPaths::GetBaseFilename(FileName);

		FString LODName = FString();
		if (BaseFileName.StartsWith(Prefix)) {
			if (InLOD.IsEmpty()) {
				return BaseFileName;
			}
			LODName = FString(TEXT("_") + InLOD.ToUpper());
			if (BaseFileName.EndsWith(LODName)) {
				return BaseFileName;
			}
		}
	}

	UE_LOG(LogRocket, Warning, TEXT("No matching asset found in directory: %s"), *ProductSourceFolderPath);
	return FString();
}

void FAssetUtilities::CreateSoundWaveAsset(const FString &SourceFolderPath, const FString &Folder) {
	const FString PackagePath = FPaths::Combine(TEXT("/Game/Rocket"), Folder);

	FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

	IFileManager &FileManager = IFileManager::Get();
	TArray<FString> Files;

	FileManager.FindFilesRecursive(Files, *SourceFolderPath, TEXT("*.wav"), true, false);

	TArray<FString> MP3Files;
	FileManager.FindFilesRecursive(MP3Files, *SourceFolderPath, TEXT("*.mp3"), true, false);
	Files.Append(MP3Files);

	TArray<UObject *> ImportedAssets;
	for (const FString &FilePath : Files) {
		FString AssetName = FPaths::GetBaseFilename(FilePath);
		FString FullPackagePath = PackagePath + "/" + AssetName;

		// Check if asset already exists
		if (FPackageName::DoesPackageExist(FullPackagePath)) {
			continue;
		}

		FString PackageName = FullPackagePath;
		UPackage *Package = CreatePackage(*PackageName);

		USoundFactory *SoundFactory = NewObject<USoundFactory>();

		constexpr EObjectFlags Flags = RF_Public | RF_Standalone;
		bool bOutOperationCanceled = false;

		if (UObject *ImportedAsset = SoundFactory->FactoryCreateFile(USoundWave::StaticClass(), Package, FName(*AssetName), Flags, *FilePath, nullptr, GWarn, bOutOperationCanceled)) {
			FAssetRegistryModule::AssetCreated(ImportedAsset);
			Package->SetDirtyFlag(true);
			ImportedAssets.Add(ImportedAsset);
		} else {
			UE_LOG(LogRocket, Error, TEXT("Asset import failed: %s"), *FilePath);
		}
	}

	for (UObject *Asset : ImportedAssets) {
		UPackage *Package = Asset->GetOutermost();
		FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());

		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SaveArgs.Error = GWarn;
		SaveArgs.bWarnOfLongFilename = false;

		if (!UPackage::SavePackage(Package, Asset, *PackageFileName, SaveArgs)) {
			UE_LOG(LogRocket, Error, TEXT("Package save failed: %s"), *Package->GetName());
		}
	}
}

void FAssetUtilities::ApplyMaterialToComponent(AActor *Actor, const FString &MaterialPath, const FString &LOD) {
	if (!Actor) {
		UE_LOG(LogRocket, Warning, TEXT("Invalid actor passed to ApplyMaterialToComponent"));
		return;
	}

	UStaticMeshComponent *MeshComponent;

	if (const AStaticMeshActor *StaticMeshActor = Cast<AStaticMeshActor>(Actor)) {
		MeshComponent = StaticMeshActor->GetStaticMeshComponent();
	} else {
		MeshComponent = Actor->FindComponentByClass<UStaticMeshComponent>();
	}

	if (!MeshComponent) {
		UE_LOG(LogRocket, Warning, TEXT("No StaticMeshComponent found on actor: %s"), *Actor->GetName());
		return;
	}

	const FString MaterialName = FindMatchingMaterialName(*MaterialPath, GetResolutionFromLOD(LOD));

	if (UMaterialInterface *Material = LoadObject<UMaterialInterface>(nullptr, *MaterialName)) {
		const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "ApplyMaterial", "Apply Material"));

		MeshComponent->Modify();

		MeshComponent->SetMaterial(0, Material);
	} else {
		UE_LOG(LogRocket, Error, TEXT("Failed to load material: %s"), *MaterialPath);
	}
}

TMap<FName, FString> FAssetUtilities::GetProductMetadata(const FAssetData& AssetData)
{
	TMap<FName, FString> Result;
    
	if (!AssetData.IsValid()) {
		UE_LOG(LogRocket, Error, TEXT("GetProductMetadata: AssetData is not valid"));
		return Result;
	}
    
	TGuardValue<bool> UnattendedScriptGuard(GIsRunningUnattendedScript, true);

	if (!RocketGeneralUtilities::IsInEditorAndNotPlaying())
	{
		return Result;
	}
	
	AssetData.EnumerateTags([&Result](TPair<FName, FAssetTagValueRef> TagPair)
	{
		Result.Add(TagPair.Key, TagPair.Value.AsString());
		return true; // Continue enumeration
	});

	return Result;
}


int32 FAssetUtilities::GetResolutionFromLOD(const FString &LOD) {
	if (LOD.Equals(TEXT("lod2"), ESearchCase::IgnoreCase)) {
		return 512;
	}
	if (LOD.Equals(TEXT("lod1"), ESearchCase::IgnoreCase)) {
		return 1024;
	}
	if (LOD.Equals(TEXT("lod0"), ESearchCase::IgnoreCase)) {
		return 2048;
	}
	if (LOD.Equals(TEXT("nanite"), ESearchCase::IgnoreCase)) {
		return 4096;
	}
	return 512;
}

ERocketProductType FAssetUtilities::GetAssetTypeFromString(const FString& ProductTypeString)
{
	if (ProductTypeString.Equals(TEXT("Asset"), ESearchCase::IgnoreCase))
		return ERocketProductType::Asset;
	if (ProductTypeString.Equals(TEXT("Environment"), ESearchCase::IgnoreCase))
		return ERocketProductType::Environment;
	if (ProductTypeString.Equals(TEXT("Material"), ESearchCase::IgnoreCase))
		return ERocketProductType::Material;
	if (ProductTypeString.Equals(TEXT("Vfx"), ESearchCase::IgnoreCase))
		return ERocketProductType::Vfx;
	if (ProductTypeString.Equals(TEXT("Sound"), ESearchCase::IgnoreCase))
		return ERocketProductType::Sound;
	if (ProductTypeString.Equals(TEXT("Decal"), ESearchCase::IgnoreCase))
		return ERocketProductType::Decal;
	if (ProductTypeString.Equals(TEXT("Tool"), ESearchCase::IgnoreCase))
		return ERocketProductType::Tool;
	if (ProductTypeString.Equals(TEXT("Bundle"), ESearchCase::IgnoreCase))
		return ERocketProductType::Bundle;
	if (ProductTypeString.Equals(TEXT("Tutorial"), ESearchCase::IgnoreCase))
		return ERocketProductType::Tutorial;
	return ERocketProductType::Unknown;
}
	
ERocketObjectType FAssetUtilities::GetObjectTypeFromString(const FString& ObjectTypeString){
	if (ObjectTypeString.Equals(TEXT("Building"), ESearchCase::IgnoreCase))
		return ERocketObjectType::Building;
	if (ObjectTypeString.Equals(TEXT("Character"), ESearchCase::IgnoreCase))
		return ERocketObjectType::Character;
	if (ObjectTypeString.Equals(TEXT("Furniture"), ESearchCase::IgnoreCase))
		return ERocketObjectType::Furniture;
	if (ObjectTypeString.Equals(TEXT("Foliage"), ESearchCase::IgnoreCase))
		return ERocketObjectType::Foliage;
	if (ObjectTypeString.Equals(TEXT("Weapon"), ESearchCase::IgnoreCase))
		return ERocketObjectType::Weapon;
	if (ObjectTypeString.Equals(TEXT("Vehicle"), ESearchCase::IgnoreCase))
		return ERocketObjectType::Vehicle;
	if (ObjectTypeString.Equals(TEXT("Prop"), ESearchCase::IgnoreCase))
		return ERocketObjectType::Prop;
	return ERocketObjectType::Unknown;
}

