// Copyright 2024 Leartes Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DragAndDrop/AssetDragDropOp.h"

DECLARE_DELEGATE_OneParam(FOnRocketAssetDrop, const FPointerEvent&);

// Class for handling Rocket asset drag and drop operations.
class FAssetDragDrop final : public FAssetDragDropOp {
public:
	DRAG_DROP_OPERATOR_TYPE(FAssetDragDrop, FAssetDragDropOp)

	// Constructor
	FAssetDragDrop() :
		MouseCursor(EMouseCursor::Default) {
	}

	// Creates a new instance of the Rocket asset drag and drop operation.
	static TSharedRef<FAssetDragDrop> New(FAssetData AssetDataArray, UActorFactory *ActorFactory, FString HTMLContent, FString Hash);

	// FDragDropOperation interface
	// Returns the default decorator for the drag and drop operation.
	virtual TSharedPtr<SWidget> GetDefaultDecorator() const override;

	// Handles the event when the asset is dragged.
	virtual void OnDragged(const FDragDropEvent &DragDropEvent) override;

	// Constructs the drag and drop operation.
	virtual void Construct() override;

	// Handles the event when the asset is dropped.
	virtual void OnDrop(bool bDropWasHandled, const FPointerEvent &MouseEvent) override;

	// Sets whether the asset can be dropped here.
	void SetCanDropHere(bool bCanDropHere);

	FOnRocketAssetDrop OnRocketAssetDropped; // Delegate for when a Rocket asset is dropped.

	FString HTMLContent; // The HTMLContent of popup.

	FString Hash; // The Hash of the asset.

	// Switches the drag and drop operation.
	static void SwitchDragDropOp(TSharedRef<FAssetDragDropOp> DragDropOperation);

	EMouseCursor::Type MouseCursor; // The cursor to display when dragging.
};
