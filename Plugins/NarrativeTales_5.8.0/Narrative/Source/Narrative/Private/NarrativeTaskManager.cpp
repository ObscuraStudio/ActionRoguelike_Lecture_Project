// Copyright Narrative Tools 2025. 


#include "NarrativeTaskManager.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "AssetRegistry/AssetData.h"
#include "NarrativeDataTask.h"

UNarrativeTaskManager::UNarrativeTaskManager()
{

}

void UNarrativeTaskManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	// Bind to the asset registry module loading to allow standalone, pie and package to all find assets
	AssetRegistryModule.Get().OnFilesLoaded().AddUObject(this, &UNarrativeTaskManager::CacheNarrativeTasks);

	// Also try immediately (works for PIE and packaged)
	CacheNarrativeTasks();
}

void UNarrativeTaskManager::Deinitialize()
{
	Super::Deinitialize();

	NarrativeTaskMap.Empty();
}

void UNarrativeTaskManager::CacheNarrativeTasks()
{
	if (NarrativeTaskMap.Num() > 0)
	{
		return; // Already cached, no need to re-run the logic
	}
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetData;

	AssetRegistryModule.Get().GetAssetsByClass(UNarrativeDataTask::StaticClass()->GetClassPathName(), AssetData);

	//AssetRegistryModule.Get().GetAssetsByPath("/Script/Narrative/NarrativeTask", AssetData);
	//Map will cache these so we can effciently access them - map means that any duplicate actions will just get overwritten which is fine
	for (auto& QuestAssetData : AssetData)
	{
		if (UNarrativeDataTask* QA = Cast<UNarrativeDataTask>(QuestAssetData.GetAsset()))
		{
			if (!NarrativeTaskMap.Contains(QA->TaskName))
			{
				FString QAEvtName = QA->TaskName;
				QAEvtName.RemoveSpacesInline();
				NarrativeTaskMap.Add(QA->TaskName, QA);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Narrative found duplicate Narrative Event assets both named %s when scanning for events. Please ensure all events are uniquely named."), *QA->GetName());
			}
		}
	}
}

class UNarrativeDataTask* UNarrativeTaskManager::GetTask(const FString& EventName) const
{
	if (NarrativeTaskMap.Contains(EventName))
	{
		return *NarrativeTaskMap.Find(EventName);
	}
	
	UE_LOG(LogTemp, Error, TEXT("Tried getting event %s but couldn't find it. (Has it been renamed?)"), *EventName);
	return nullptr;
}
