//Easy Multi Save - Copyright (C) 2026 by Michael Hegemann.  

#pragma once

#include "EMSObjectBase.h"
#include "EMSObjectAdv.generated.h"

class FBufferArchive;
class FMemoryReader;
class FMemoryWriter;
class UWorld;


UCLASS(BlueprintType, meta = (DisplayName = "Easy Multi Save Adv.", Keywords = "Save, EMS, EasyMultiSave, EasySave"))
class EASYMULTISAVE_API UEMSObjectAdv : public UEMSObjectBase
{
	GENERATED_BODY()

protected:

	//Override here, as it would call twice(EMSObject already calls the one from EMSObjectBase)
	void Initialize(FSubsystemCollectionBase& Collection) override {}
	void Deinitialize() override {}

/** Object Collection  */

public:

	/**
	Save a Collection of UObjects to a single file. 
	Provides an efficient way to store any UObject directly.Useful for Subsystems, GameInstance etc.
	 
	@param Objects     Array of serialized object states to write.
	@param bUseSlot    True to save inside the current slot directory.
	@param FileName    Custom filename identifier for the collection. Defaults to 'ObjectCollection.sav' .
	@return            True on success.
	*/
	UFUNCTION(BlueprintCallable, Category = "Easy Multi Save | Object Collection", meta = (AdvancedDisplay = "FileName"))
	bool SaveObjectCollection(const TArray<FRawObjectSaveData>& Objects, bool bUseSlot, FString FileName);

	/**
	Load a Collection of UObjects. 
	Provides an efficient way to store any UObject directly. Useful for Subsystems, GameInstance etc.
	
	@param Objects     Array of serialized object states to load on demand.
	@param bUseSlot    True to save inside the current slot directory.
	@param FileName    Custom filename identifier for the collection. Defaults to 'ObjectCollection.sav' .
	@return            True on success.
	*/
	UFUNCTION(BlueprintCallable, Category = "Easy Multi Save | Object Collection", meta = (AdvancedDisplay = "FileName"))
	bool LoadObjectCollection(const TArray<FRawObjectSaveData>& Objects, bool bUseSlot, FString FileName);

private:

	virtual bool ProcessObjectCollection(FMemoryReader& FromBinary, const FSaveObjects& Objects) const override;
	FString GetObjectCollectionPath(const bool bUseSlot, const FString& CollectionFileName) const;

/** Native Cloud Saving */

public:

	/**
	Exports save data to byte array. Uses the current Save Slot by default. 
	This can be used with Epic Online Services or similar cloud storage providers.
	
	@param FileType    The category to export.
	@param CustomFiles Array of Custom file names to pack. Can be added to the Slot as well. 
	@param OutByteData The resulting raw binary data.
	*/
	UFUNCTION(BlueprintCallable, Category = "Easy Multi Save | Cloud", meta = (AdvancedDisplay = "CustomFiles", AutoCreateRefTerm = "CustomFiles"))
	bool ExportCloudData(ECloudFileType FileType, const TArray<FString>& CustomFiles, TArray<uint8>& ByteData);

	/**
	Imports save data from byte array. Uses the current Save Slot by default. 
	You can normally Load the data afterwards. Paths are resolved automatically.
	This can be used with Epic Online Services or similar cloud storage providers.
	
	@param ByteData   Raw data downloaded from the cloud.
	@param FileType   The category being restored. Must match the export FileType.
	*/
	UFUNCTION(BlueprintCallable, Category = "Easy Multi Save | Cloud")
	bool ImportCloudData(const TArray<uint8>& ByteData, ECloudFileType FileType);

private:

	bool ReadDiskFile(const FString& FilePath, TArray<uint8>& OutData) const;
	bool WriteDiskFile(const FString& FilePath, const TArray<uint8>& InData);
	FString GetCloudFilePath(const FCloudFilePath& Path) const;
};