// Copyright 2024 Leartes Studios. All Rights Reserved.

#pragma once

#include <functional>

#include "CoreMinimal.h"
#include "NetworkDataContext.h"
#include "Templates/SharedPointer.h"
#include "UObject/WeakObjectPtr.h"

DECLARE_DELEGATE_OneParam(FOnAssetProcessingCompleted, const FString& /* Hash*/)

/**
 * FTempAssetHandler - Handles temporary assets that represent a downloaded asset in progress.
 */
class FTempAssetHandler : public TSharedFromThis<FTempAssetHandler> {
public:
	using FCallback = std::function<void()>;

	/** Constructor */
	FTempAssetHandler(AActor *InActor, const FRocketProductData& InProductData);

	/** Returns a shared pointer to this instance */
	TSharedPtr<FTempAssetHandler> AsSharedPtr();

	/** Accessor for TempActor */
	TArray<TWeakObjectPtr<AActor>> GetTempActors() const;

	/** Accessor for TargetAssetName */
	FString GetTargetAssetName() const;

	/** Accessor for Hash */
	FString GetHash() const;

	/** Accessor for LOD */
	FString GetLOD() const;

	/** Accessor for ProductName */
	FString GetProductName() const;

	/** Accessor for HTMLContent */
	FString GetAssetType() const;

	/** Accessor for HTMLContent */
	FString GetObjectType() const;

	/** Accessor for HTMLContent */
	bool GetIsInteractive() const;

	const FRocketProductData& GetProductData() const;

	/** Accessor for World */
	UWorld *GetWorld() const;

	/** Setter for TempActor */
	void AddNewTempActor(AActor *InActor);
	
	/** Accessor for HTMLContent */
	FString GetHTMLContent() const;

	void SetCallback(const FOnAssetProcessingCompleted &InCallback);

	void CallCallback() const;

private:
	FOnAssetProcessingCompleted OnAssetProcessingCompleted;

	/** Weak pointer to the temporary actor */
	TArray<TWeakObjectPtr<AActor>> TempActors;

	FRocketProductData ProductData;
};
