//Easy Multi Save - Copyright (C) 2026 by Michael Hegemann.  

#include "EMSObjectAdv.h"
#include "EMSDebug.h"
#include "EMSPaths.h"
#include "EMSVersion.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/BufferArchive.h"

/**
Object Collection
This is kept seperate from Actor Saving.
It will also provide the base for future Unreal features like SceneGraph.
**/

bool UEMSObjectAdv::SaveObjectCollection(const TArray<FRawObjectSaveData>& Objects, bool bUseSlot, FString FileName)
{
	if (Objects.IsEmpty())
	{
		return false;
	}

	const FString FullPath = GetObjectCollectionPath(bUseSlot, FileName);

	//Add valid objects
	TMap<FString, TArray<uint8>> Serialized;
	for (const FRawObjectSaveData& Entry : Objects)
	{
		if (!Entry.IsValidData())
		{
			continue;
		}

		TArray<uint8> Data;
		FStructHelpers::SerializeStruct(Entry.Object);
		SerializeToBinary(Entry.Object, Data, FSerializeMeta::Collection());

		//Save Report for collection
		WriteSaveReport(FSaveReportContext::ForCollection(Entry.Object, FileName, bUseSlot, false));

		if (!Data.IsEmpty())
		{
			Serialized.Add(Entry.Id, MoveTemp(Data));
		}
	}

	if (Serialized.IsEmpty())
	{
		return false;
	}

	FBufferArchive Archive;
	Archive << Serialized;

	//Check for error and proceed
	bool bSuccess = false;
	if (!FSaveHelpers::HasSaveArchiveError(Archive, ESaveErrorType::ER_Collection))
	{
		bSuccess = SaveBinaryArchive(*FullPath, Archive);
	}

	if (bSuccess)
	{
		//Update slot, since data in it was modified
		if (bUseSlot)
		{
			SaveSlotInfoObject(GetCurrentSaveGameName());
		}

		const FString SlotMessage = bUseSlot ? TEXT("to Slot") : TEXT("");
		UE_LOG(LogEasyMultiSave, Log, TEXT("Object Collection saved %s"), *SlotMessage);
	}

	return bSuccess;
}

bool UEMSObjectAdv::LoadObjectCollection(const TArray<FRawObjectSaveData>& Objects, bool bUseSlot, FString FileName)
{
	const FString FullPath = GetObjectCollectionPath(bUseSlot, FileName);

	FSaveObjects AllObjects;
	AllObjects.AddFromRaw(Objects);

	//Load from top level Binary archive as usual
	const FLoadArchiveContext Context = FLoadArchiveContext(FullPath, EDataLoadType::DATA_Collection, AllObjects);
	const bool bLoadBinary = LoadBinaryArchive(Context);

	if (bLoadBinary)
	{
		const FString SlotMessage = bUseSlot ? TEXT("from Slot") : TEXT("");
		UE_LOG(LogEasyMultiSave, Log, TEXT("Object Collection loaded %s"), *SlotMessage);

		//Save Report on load post serialization from LoadBinaryArchive() -> ProcessObjectCollection()
		if (FSettingHelpers::IsSaveReportEnabled())
		{
			for (const FRawObjectSaveData& Entry : Objects)
			{
				if (Entry.IsValidData())
				{
					WriteSaveReport(FSaveReportContext::ForCollection(Entry.Object, FileName, bUseSlot, true));
				}
			}
		}
	}

	return bLoadBinary;
}

bool UEMSObjectAdv::ProcessObjectCollection(FMemoryReader& FromBinary, const FSaveObjects& Objects) const
{
	//Deserialze as same type
	TMap<FString, TArray<uint8>> DataWithId;
	FromBinary << DataWithId;

	if (DataWithId.IsEmpty())
	{
		return false;
	}

	bool bAnyLoaded = false;
	for (const FRawObjectSaveData& Entry : Objects.GetRawData())
	{
		if (!Entry.IsValidData())
		{
			continue;
		}

		//Find the Object by Id
		const TArray<uint8>* Binary = DataWithId.Find(Entry.Id);
		if (!Binary || Binary->IsEmpty())
		{
			continue;
		}

		FStructHelpers::SerializeStruct(Entry.Object);
		SerializeFromBinary(Entry.Object, *Binary, FSerializeMeta::Collection());

		bAnyLoaded = true;
	}

	return bAnyLoaded;
}

FString UEMSObjectAdv::GetObjectCollectionPath(const bool bUseSlot, const FString& CollectionFileName) const
{
	const FString FileName = CollectionFileName.IsEmpty() ? EMS::ObjectCollection : CollectionFileName;
	const FString SlotName = bUseSlot ? GetCurrentSaveGameName() : FString();
	const FString FullPath = CustomSaveFile(FileName, SlotName);

	return FullPath;
}

/**
Native Cloud Saving
Will convert save files to byte arrays and back.
This is for use with Epic Online Services and similar.
**/

static bool IsBlueprintCloudType(const ECloudFileType FileType)
{
	return (FileType == ECloudFileType::FullSlot || FileType == ECloudFileType::Custom || FileType == ECloudFileType::CustomSlot);
}

bool UEMSObjectAdv::ExportCloudData(ECloudFileType FileType, const TArray<FString>& CustomFiles, TArray<uint8>& ByteData)
{
	if (IsAsyncSaveOrLoadTaskActive())
	{
		UE_LOG(LogEasyMultiSave, Warning, TEXT("Cloud: Cannot export data while save or load is active"));
		return false;
	}

	ByteData.Empty();

	//Handle Manifest Packing for Multiple Files
	if (IsBlueprintCloudType(FileType))
	{
		TArray<FCloudArchiveEntry> Entries;

		//Pack Core Files if FullSlot
		if (FileType == ECloudFileType::FullSlot)
		{
			struct FSlotFile
			{
				const FString& Key;
				ECloudFileType Type;
			};

			const TArray<FSlotFile> CoreFiles =
			{
				{ EMS::PlayerSuffix, ECloudFileType::Player   },
				{ EMS::ActorSuffix,  ECloudFileType::Level    },
				{ EMS::SlotSuffix,   ECloudFileType::SlotInfo }
			};

			for (const FSlotFile& File : CoreFiles)
			{
				const FString Path = GetCloudFilePath(FCloudFilePath(File.Type, FString()));

				FCloudArchiveEntry Entry;
				Entry.FileKey = File.Key;

				if (ReadDiskFile(Path, Entry.Data))
				{
					Entries.Add(MoveTemp(Entry));
				}
			}
		}

		//If FullSlot, assume extra files are in the slot. Otherwise use the specified Custom type.
		const ECloudFileType CustomOverride = (FileType == ECloudFileType::FullSlot) ? ECloudFileType::CustomSlot : FileType;
		for (const FString& CustomName : CustomFiles)
		{
			if (CustomName.IsEmpty())
			{
				continue;
			}

			const FString Path = GetCloudFilePath(FCloudFilePath(CustomOverride, CustomName));

			FCloudArchiveEntry Entry;
			Entry.FileKey = CustomName;

			if (ReadDiskFile(Path, Entry.Data))
			{
				Entries.Add(MoveTemp(Entry));
			}
		}

		if (Entries.IsEmpty())
		{
			return false;
		}

		//Write packed manifest to output
		FMemoryWriter Writer(ByteData, true);
		Writer << Entries;

		return true;
	}

	//Handle Single Core File Export (Player, Level, SlotInfo)
	const FString FilePath = GetCloudFilePath(FCloudFilePath(FileType, FString()));
	if (FilePath.IsEmpty())
	{
		return false;
	}

	return ReadDiskFile(FilePath, ByteData);
}

bool UEMSObjectAdv::ImportCloudData(const TArray<uint8>& ByteData, ECloudFileType FileType)
{
	if (IsAsyncSaveOrLoadTaskActive())
	{
		UE_LOG(LogEasyMultiSave, Warning, TEXT("Cloud: Cannot import data while save or load is active"));
		return false;
	}

	if (ByteData.IsEmpty())
	{
		UE_LOG(LogEasyMultiSave, Warning, TEXT("Cloud: Import data is empty"));
		return false;
	}

	//Ensure the base slot directory exists for any write operation
	const FString SlotName = GetCurrentSaveGameName();
	if (!VerifyOrCreateDirectory(SlotName))
	{
		return false;
	}

	//Handle Manifest Unpacking for Multiple Files
	if (IsBlueprintCloudType(FileType))
	{
		TArray<FCloudArchiveEntry> Entries;
		FMemoryReader Reader(ByteData, true);
		Reader << Entries;

		if (Reader.IsError() || Entries.IsEmpty())
		{
			UE_LOG(LogEasyMultiSave, Error, TEXT("Cloud: Failed to parse manifest blob"));
			return false;
		}

		bool bAllSuccess = true;
		for (FCloudArchiveEntry& Entry : Entries)
		{
			ECloudFileType EntryType = FileType;

			//If FullSlot, we must dynamically figure out if this entry is a core file or a custom slot file
			if (FileType == ECloudFileType::FullSlot)
			{
				if (Entry.FileKey == EMS::PlayerSuffix)		 
					EntryType = ECloudFileType::Player;
				else if (Entry.FileKey == EMS::ActorSuffix)  
					EntryType = ECloudFileType::Level;
				else if (Entry.FileKey == EMS::SlotSuffix)   
					EntryType = ECloudFileType::SlotInfo;
				else                                         
					EntryType = ECloudFileType::CustomSlot;
			}

			//The FileKey perfectly maps to the file name
			const FString Path = GetCloudFilePath(FCloudFilePath(EntryType, Entry.FileKey));

			if (!WriteDiskFile(Path, Entry.Data))
			{
				bAllSuccess = false;
			}
		}

		return bAllSuccess;
	}

	//Handle single file import
	const FString FilePath = GetCloudFilePath(FCloudFilePath(FileType, FString()));
	if (FilePath.IsEmpty())
	{
		return false;
	}

	return WriteDiskFile(FilePath, ByteData);
}

bool UEMSObjectAdv::ReadDiskFile(const FString& FilePath, TArray<uint8>& OutData) const
{
	const FFileContext Context(PlayerIndex, FilePath);
	if (FSaveFileIntegrity::LoadRawSaveData(Context, OutData) != EFileValidity::FILE_VALID)
	{
		UE_LOG(LogEasyMultiSave, Error, TEXT("Cloud: Corrupt or unreadable data at '%s'"), *FilePath);
		return false;
	}

	return true;
}

bool UEMSObjectAdv::WriteDiskFile(const FString& FilePath, const TArray<uint8>& InData)
{
	//This means the file is broken, maybe sync error or something.
	FSaveHeader TempHeader;
	if (!FSaveFileIntegrity::IsFileHashValid(InData, TempHeader))
	{
		UE_LOG(LogEasyMultiSave, Error, TEXT("Cloud: Integrity check failed! Aborting write to '%s'"), *FilePath);
		return false;
	}

	const FFileContext Context(PlayerIndex, FilePath);
	if (!FSaveFileIntegrity::SaveToFile(Context, InData))
	{
		UE_LOG(LogEasyMultiSave, Error, TEXT("Cloud: Failed to write file '%s'"), *FilePath);
		return false;
	}

	UE_LOG(LogEasyMultiSave, Log, TEXT("Cloud: Wrote %d bytes to '%s'"), InData.Num(), *FilePath);
	return true;
}

FString UEMSObjectAdv::GetCloudFilePath(const FCloudFilePath& Path) const
{
	const FString SlotName = GetCurrentSaveGameName();

	switch (Path.Type)
	{
	case ECloudFileType::Player:
		return PlayerSaveFile(SlotName);

	case ECloudFileType::Level:
		return ActorSaveFile(SlotName);

	case ECloudFileType::SlotInfo:
		return SlotInfoSaveFile(SlotName);

	case ECloudFileType::Custom:
		return CustomSaveFile(Path.FileName, FString());

	case ECloudFileType::CustomSlot:
		return CustomSaveFile(Path.FileName, SlotName);

	default:
		return FString();
	}
}