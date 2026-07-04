// Copyright 2024 Leartes Studios. All Rights Reserved.

#include "PathHelpers.h"
#include "NetworkDataContext.h"
#include "RocketModule.h"
#include "Algo/Count.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

FString FPathHelpers::GetHomeDirectory() {
#if PLATFORM_WINDOWS
	return FPlatformMisc::GetEnvironmentVariable(TEXT("USERPROFILE"));
#elif PLATFORM_MAC || PLATFORM_LINUX
	return FPlatformMisc::GetEnvironmentVariable(TEXT("HOME"));
#endif
}

FString FPathHelpers::GetRocketDepotPath() {
	FString UserDirectory = GetHomeDirectory();
	FString DepotPath;

#if PLATFORM_WINDOWS
	DepotPath = FPaths::Combine(UserDirectory, TEXT("AppData"), TEXT("Roaming"), TEXT("Rocket"));
#elif PLATFORM_MAC
	DepotPath = FPaths::Combine(UserDirectory, TEXT("Library"), TEXT("Application Support"), TEXT("Rocket"));
#elif PLATFORM_LINUX
	DepotPath = FPaths::Combine(UserDirectory, TEXT(".local"), TEXT("share"), TEXT("Rocket"));
#endif

	FString FullDepotPath = FPaths::ConvertRelativePathToFull(DepotPath);

	return FullDepotPath;
}

FString FPathHelpers::GetDefaultRocketExportPath() {
	return FPaths::Combine(GetHomeDirectory(), TEXT("Documents"), TEXT("Rocket"));
}

FString FPathHelpers::GetRocketProductsPath() {
	return FPaths::Combine(GetRocketDepotPath(), TEXT("products"));
}

FString FPathHelpers::GetRocketCompressedPath() {
	return FPaths::Combine(GetRocketDepotPath(), TEXT("compressed"));
}

FString FPathHelpers::GetDefaultProjectPath() {
	FString DefaultPath = FPaths::Combine(GetHomeDirectory(), TEXT("Documents"), TEXT("Unreal Projects"));
	return DefaultPath;
}

FString FPathHelpers::GetProductImportableContentPath(const FString &InProductsName, const FString &Hash, const ERocketProductType &Type = ERocketProductType::Unknown) {
	FString HashCombinedProductName = InProductsName + TEXT("_") + Hash;
	if (Type == ERocketProductType::Sound) {
		return FPaths::Combine(GetRocketProductsPath(), HashCombinedProductName, InProductsName);
	}
	return FPaths::Combine(GetRocketProductsPath(), HashCombinedProductName, InProductsName, TEXT("Unreal"), TEXT("Rocket"));
}

TArray<FString> FPathHelpers::GetTopLevelFolders(const FString &ProductRocketPath) {
	TArray<FString> FolderNames;
	IFileManager &FileManager = IFileManager::Get();
	FileManager.FindFilesRecursive(FolderNames, *ProductRocketPath, TEXT("*"), false, true);

	FolderNames.RemoveAll([&ProductRocketPath](const FString &FolderPath) {
		return FolderPath.Contains(FPaths::Combine(ProductRocketPath, TEXT("Masters")));
	});

	TArray<FString> TopLevelFolders;
	for (const FString &FolderPath : FolderNames) {
		FString RelativePath = FolderPath;
		FPaths::MakePathRelativeTo(RelativePath, *(ProductRocketPath + "/"));

		int32 SlashIndex;
		if (RelativePath.FindChar(TEXT('/'), SlashIndex)) {
			FString TopLevelFolder = RelativePath.Left(SlashIndex);
			TopLevelFolders.AddUnique(TopLevelFolder);
		} else {
			TopLevelFolders.AddUnique(RelativePath);
		}
	}

	return TopLevelFolders;
}

bool FPathHelpers::IsTheEnvironmentImportedBefore(const FString &InEnvironment) {
	int32 UnderscoreIndex;
	if (InEnvironment.FindLastChar(TEXT('_'), UnderscoreIndex)) {
		FString PotentialVersion = InEnvironment.Mid(UnderscoreIndex + 1);
		TArray<FString> VersionParts;
		PotentialVersion.ParseIntoArray(VersionParts, TEXT("."), true);

		if (VersionParts.Num() == 2 && VersionParts[0].IsNumeric() && VersionParts[1].IsNumeric()) {
			FString EnvironmentNameWithoutVersion = InEnvironment.Left(UnderscoreIndex);
			FString ContentFolderPath = FPaths::ProjectContentDir();
			FString EnvironmentFolderPath = FPaths::Combine(ContentFolderPath, EnvironmentNameWithoutVersion);
			IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
			
			if (PlatformFile.DirectoryExists(*EnvironmentFolderPath)) {
				UE_LOG(LogRocket, Warning, TEXT("The environment '%s' has already been imported."), *EnvironmentNameWithoutVersion);
				return true; 
			}

			FString SanitizedEnvironmentName = EnvironmentNameWithoutVersion.Replace(TEXT("_"), TEXT(""));
			EnvironmentFolderPath = FPaths::Combine(ContentFolderPath, SanitizedEnvironmentName);
			
			if (PlatformFile.DirectoryExists(*EnvironmentFolderPath)) {
				UE_LOG(LogRocket, Warning, TEXT("The environment '%s' has already been imported."), *EnvironmentNameWithoutVersion);
				return true; 
			}
		}
	}
	return false;
}

bool FPathHelpers::IsAnyEnvSourceFileImportedBefore(const FString &InEnvironment) {

	FString SourceFolder = FPaths::Combine(*GetRocketDepotPath(), TEXT("products"), InEnvironment, TEXT("Content"));

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (!PlatformFile.DirectoryExists(*SourceFolder)) {
		UE_LOG(LogRocket, Error, TEXT("Source folder does not exist: %s"), *SourceFolder);
		return false;
	}

	TArray<FString> SourceFiles;
	PlatformFile.FindFilesRecursively(SourceFiles, *SourceFolder, TEXT("uasset"));

	if (SourceFiles.Num() == 0) {
		UE_LOG(LogRocket, Error, TEXT("No files found in the source folder: %s"), *SourceFolder);
		return false;
	}

	FString ContentFolderPath = FPaths::ProjectContentDir();
	for (const FString& SourceFile : SourceFiles) {
		FString RelativeFilePath = SourceFile.Mid(SourceFolder.Len() + 1);
		FString DestinationPath = FPaths::Combine(ContentFolderPath, RelativeFilePath);
		
		if (PlatformFile.FileExists(*DestinationPath)) {
			return true;
		}
	}
	return false;
}

FString FPathHelpers::FindMainFolderNameForProduct(const FString &InProductsName, const FString &Hash, const ERocketProductType &Type) {

	const FString ProductRocketPath = GetProductImportableContentPath(InProductsName, Hash, Type);
	if (!FPaths::DirectoryExists(ProductRocketPath)) {
		UE_LOG(LogRocket, Log, TEXT("Directory does not exist: %s"), *ProductRocketPath);
		return {};
	}

	if (Type == ERocketProductType::Sound) {
		return InProductsName;
	}

	TArray<FString> TopLevelFolders = GetTopLevelFolders(ProductRocketPath);
	return TopLevelFolders.Num() > 0 ? TopLevelFolders[0] : FString();
}

FString FPathHelpers::AssetDirectory(const FString &InProductsName, const FString &Hash, const ERocketProductType &InType) {
	return GetProductImportableContentPath(InProductsName, Hash, InType);
}

FString FPathHelpers::CombineProductNameWithHash(const FString &InProductsName, const FString &Hash, const ERocketProductType &InType) {
	const FString ProductRocketPath = GetProductImportableContentPath(InProductsName, Hash, InType);

	if (InType == ERocketProductType::Sound) {
		return InProductsName + TEXT("_") + Hash + '/' + InProductsName;
	}

	TArray<FString> TopLevelFolders = GetTopLevelFolders(ProductRocketPath);
	return TopLevelFolders.Num() > 0 ? TopLevelFolders[0] + TEXT("_") + Hash + '/' + TopLevelFolders[0] : FString();
}

bool HasValidRoot(const FString &ObjectPath) {
	FString Filename;
	bool bValidRoot;
	if (!ObjectPath.IsEmpty() && ObjectPath[ObjectPath.Len() - 1] == TEXT('/')) {
		bValidRoot = FPackageName::TryConvertLongPackageNameToFilename(ObjectPath, Filename);
	} else {
		FString ObjectPathWithSlash = ObjectPath;
		ObjectPathWithSlash.AppendChar(TEXT('/'));
		bValidRoot = FPackageName::TryConvertLongPackageNameToFilename(ObjectPathWithSlash, Filename);
	}

	return bValidRoot;
}

bool IsAValidPathForCreateNewAsset(const FString &ObjectPath, FString &OutFailureReason) {
	const FString ObjectName = FPackageName::ObjectPathToObjectName(ObjectPath);

	// Make sure the name is not already a class or otherwise invalid for saving
	FText FailureReason;
	if (!FFileHelper::IsFilenameValidForSaving(ObjectName, FailureReason)) {
		OutFailureReason = FailureReason.ToString();
		return false;
	}

	// Make sure the new name only contains valid characters
	if (!FName::IsValidXName(ObjectName, INVALID_OBJECTNAME_CHARACTERS INVALID_LONGPACKAGE_CHARACTERS, &FailureReason)) {
		OutFailureReason = FailureReason.ToString();
		return false;
	}

	// Make sure we are not creating an FName that is too large
	if (ObjectPath.Len() >= NAME_SIZE) {
		OutFailureReason = TEXT("This asset name is too long (") + FString::FromInt(ObjectPath.Len()) + TEXT(" characters), the maximum is ") + FString::FromInt(NAME_SIZE) + TEXT(". Please choose a shorter name.");
		return false;
	}

	FAssetRegistryModule &AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1)
	FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(ObjectPath);
#else
	FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(*ObjectPath);
#endif

	if (AssetData.IsValid()) {
		OutFailureReason = TEXT("An asset already exists at this location.");
		return false;
	}

	return true;
}

/** Remove Class from "Class /Game/MyFolder/MyAsset" */
FString RemoveFullName(const FString &AnyAssetPath, FString &OutFailureReason) {
	FString Result = AnyAssetPath.TrimStartAndEnd();
	int32 NumberOfSpace = Algo::Count(AnyAssetPath, TEXT(' '));

	if (NumberOfSpace == 0) {
		return MoveTemp(Result);
	}
	if (NumberOfSpace > 1) {
		OutFailureReason = FString::Printf(TEXT("Can't convert path '%s' because there are too many spaces."), *AnyAssetPath);
		return FString();
	}

	int32 FoundIndex = 0;
	AnyAssetPath.FindChar(TEXT(' '), FoundIndex);
	check(FoundIndex > INDEX_NONE && FoundIndex < AnyAssetPath.Len());

	// Confirm that it's a valid Class
	FString ClassName = AnyAssetPath.Left(FoundIndex);

	// Convert \ to /
	ClassName.ReplaceInline(TEXT("\\"), TEXT("/"), ESearchCase::CaseSensitive);

	// Test ClassName for invalid Char
	const int32 StrLen = FCString::Strlen(INVALID_OBJECTNAME_CHARACTERS);
	for (int32 Index = 0; Index < StrLen; ++Index) {
		int32 InvalidFoundIndex = 0;
		if (ClassName.FindChar(INVALID_OBJECTNAME_CHARACTERS[Index], InvalidFoundIndex)) {
			OutFailureReason = FString::Printf(TEXT("Can't convert the path %s because it contains invalid characters (probably spaces)."), *AnyAssetPath);
			return FString();
		}
	}

	// Return the path without the Class name
	return AnyAssetPath.Mid(FoundIndex + 1);
}

bool IsAValidPath(const FString &Path, const TCHAR *InvalidChar, FString &OutFailureReason) {
	const int32 StrLen = FCString::Strlen(InvalidChar);
	for (int32 Index = 0; Index < StrLen; ++Index) {
		int32 FoundIndex = 0;
		if (Path.FindChar(InvalidChar[Index], FoundIndex)) {
			OutFailureReason = FString::Printf(TEXT("Can't convert the path %s because it contains invalid characters."), *Path);
			return false;
		}
	}

	if (Path.Len() > FPlatformMisc::GetMaxPathLength()) {
		OutFailureReason = FString::Printf(TEXT("Can't convert the path because it is too long (%d characters). This may interfere with cooking for consoles. Unreal filenames should be no longer than %d characters. Full path value: %s"), Path.Len(), FPlatformMisc::GetMaxPathLength(), *Path);
		return false;
	}
	return true;
}

FString ConvertAnyPathToLongPackagePath(const FString &AnyPath, FString &OutFailureReason) {
	if (AnyPath.Len() < 2) // minimal length to have /G
	{
		OutFailureReason = FString::Printf(TEXT("Can't convert the path '%s' because the Root path need to be specified. ie /Game/"), *AnyPath);
		return FString();
	}

	FString TextPath = FPackageName::ExportTextPathToObjectPath(AnyPath);

	// Remove class name Fullname
	TextPath = RemoveFullName(TextPath, OutFailureReason);
	if (TextPath.IsEmpty()) {
		return FString();
	}

	// Convert \ to /
	TextPath.ReplaceInline(TEXT("\\"), TEXT("/"), ESearchCase::CaseSensitive);
	FPaths::RemoveDuplicateSlashes(TextPath);

	{
		// Remove .
		int32 ObjectDelimiterIdx;
		if (TextPath.FindChar(TEXT('.'), ObjectDelimiterIdx)) {
			TextPath.LeftInline(ObjectDelimiterIdx);
		}

		// Remove :
		if (TextPath.FindChar(TEXT(':'), ObjectDelimiterIdx)) {
			TextPath.LeftInline(ObjectDelimiterIdx);
		}
	}

	// Test for invalid characters
	if (!IsAValidPath(TextPath, INVALID_LONGPACKAGE_CHARACTERS, OutFailureReason)) {
		return FString();
	}

	// Confirm that we have a valid Root Package and get the valid PackagePath /Game/MyFolder
	FString PackagePath;
	if (!FPackageName::TryConvertFilenameToLongPackageName(TextPath, PackagePath, &OutFailureReason)) {
		return FString();
	}

	if (PackagePath.Len() == 0) {
		OutFailureReason = FString::Printf(TEXT("Can't convert the path '%s' because of an internal error. TryConvertFilenameToLongPackageName should have return false."), *AnyPath);
		return FString();
	}

	if (PackagePath[0] != TEXT('/')) {
		OutFailureReason = FString::Printf(TEXT("Can't convert path '%s' because the PackagePath '%s' doesn't start with a '/'."), *AnyPath, *PackagePath);
		return FString();
	}

	if (PackagePath[PackagePath.Len() - 1] == TEXT('/')) {
		PackagePath.RemoveAt(PackagePath.Len() - 1);
	}

	if (FPackageName::IsScriptPackage(PackagePath)) {
		OutFailureReason = FString::Printf(TEXT("Can't convert the path '%s' because it starts with /Script/"), *AnyPath);
		return FString();
	}
	if (FPackageName::IsMemoryPackage(PackagePath)) {
		OutFailureReason = FString::Printf(TEXT("Can't convert the path '%s' because it starts with /Memory/"), *AnyPath);
		return FString();
	}

	// Confirm that the PackagePath start with a valid root
	if (!HasValidRoot(PackagePath)) {
		OutFailureReason = FString::Printf(TEXT("Can't convert the path '%s' because it does not map to a root."), *AnyPath);
		return FString();
	}

	return PackagePath;
}

bool FPathHelpers::DoesAssetDirectoryExist(const FString &InDirectory) {
	TGuardValue<bool> UnattendedScriptGuard(GIsRunningUnattendedScript, true);

	FString FailureReason;
	FString ValidDirectoryPath = ConvertAnyPathToLongPackagePath(InDirectory, FailureReason);
	if (ValidDirectoryPath.IsEmpty()) {
		UE_LOG(LogRocket, Error, TEXT("DoesDirectoryExists. Failed to convert the path. %s"), *FailureReason);
		return false;
	}

	FString FilePath = FPaths::ConvertRelativePathToFull(FPackageName::LongPackageNameToFilename(ValidDirectoryPath + TEXT("/")));
	if (FilePath.IsEmpty()) {
		UE_LOG(LogRocket, Error, TEXT("DoesDirectoryExists. Failed to convert the path '%s' to a full path. Was it rooted?"), *ValidDirectoryPath);
		return false;
	}

	bool bResult = IFileManager::Get().DirectoryExists(*FilePath);
	if (bResult) {
		// The folder may not exist in the ContentBrowser
		FAssetRegistryModule &AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		AssetRegistryModule.Get().AddPath(ValidDirectoryPath);
	}

	return bResult;
}
