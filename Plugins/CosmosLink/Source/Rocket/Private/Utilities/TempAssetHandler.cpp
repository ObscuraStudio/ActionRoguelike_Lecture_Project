// Copyright 2024 Leartes Studios. All Rights Reserved.


#include "TempAssetHandler.h"

FTempAssetHandler::FTempAssetHandler(AActor *InActor, const FRocketProductData &InProductData) :
	ProductData(InProductData) {
	if (IsValid(InActor)) {
		TempActors.Add(InActor);
	}
}

TSharedPtr<FTempAssetHandler> FTempAssetHandler::AsSharedPtr() {
	return AsShared();
}

TArray<TWeakObjectPtr<AActor>> FTempAssetHandler::GetTempActors() const {
	return TempActors;
}

FString FTempAssetHandler::GetTargetAssetName() const {
	return ProductData.TargetAssetName;
}

FString FTempAssetHandler::GetHash() const {
	return ProductData.Hash;
}

FString FTempAssetHandler::GetLOD() const {
	return ProductData.LOD;
}

FString FTempAssetHandler::GetProductName() const {
	return ProductData.ProductName;
}

FString FTempAssetHandler::GetAssetType() const {
	return ProductData.AssetType;
}

FString FTempAssetHandler::GetObjectType() const {
	return ProductData.ObjectType;
}

bool FTempAssetHandler::GetIsInteractive() const {
	return ProductData.bIsInteractive;
}

const FRocketProductData& FTempAssetHandler::GetProductData() const{
	return ProductData;
}

FString FTempAssetHandler::GetHTMLContent() const {
	return ProductData.HTMLContent;
}

UWorld *FTempAssetHandler::GetWorld() const {
	for (auto CurrentActor : GetTempActors()) {
		if (CurrentActor.IsValid() && IsValid(CurrentActor->GetWorld())) {
			return CurrentActor->GetWorld();
		}
	}
	return nullptr;
}

void FTempAssetHandler::AddNewTempActor(AActor *InActor) {
	if (IsValid(InActor)) {
		TempActors.Add(InActor);
	}
}

void FTempAssetHandler::SetCallback(const FOnAssetProcessingCompleted &InCallback) {
	OnAssetProcessingCompleted = InCallback;
}

void FTempAssetHandler::CallCallback() const {
	if (OnAssetProcessingCompleted.IsBound()) {
		OnAssetProcessingCompleted.Execute(GetHash());
	}
}
