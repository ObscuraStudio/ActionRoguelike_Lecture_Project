// Copyright 2024 Leartes Studios. All Rights Reserved.

#include "AssetDragDrop.h"
#include "AssetUtilities.h"
#include "FileDownloadHandler.h"
#include "Rocket.h"
#include "SWebBrowser.h"

inline TSharedPtr<SWidget> FAssetDragDrop::GetDefaultDecorator() const {
	const TSharedPtr<SWebBrowser> PopupWebBrowser = SNew(SWebBrowser).ShowControls(false).SupportsTransparency(true).BackgroundColor(FColor::Transparent);

	if (!PopupWebBrowser->IsLoaded()) {
		PopupWebBrowser->LoadString(HTMLContent, TEXT(""));
	}

	return SNew(SBox).HeightOverride(150).WidthOverride(150)[PopupWebBrowser.ToSharedRef()];
}

void FAssetDragDrop::OnDragged(const FDragDropEvent &DragDropEvent) {
	if (CursorDecoratorWindow.IsValid()) {
		CursorDecoratorWindow->MoveWindowTo(DragDropEvent.GetScreenSpacePosition());
	}
}

void FAssetDragDrop::Construct() {
	MouseCursor = EMouseCursor::GrabHandClosed;

	FDragDropOperation::Construct();
}

void FAssetDragDrop::OnDrop(bool bDropWasHandled, const FPointerEvent &MouseEvent) {

	OnRocketAssetDropped.ExecuteIfBound(MouseEvent);
}

void FAssetDragDrop::SetCanDropHere(bool bCanDropHere) {
	MouseCursor = bCanDropHere ? EMouseCursor::TextEditBeam : EMouseCursor::SlashedCircle;
}

void FAssetDragDrop::SwitchDragDropOp(TSharedRef<FAssetDragDropOp> DragDropOperation) {
	if (!IsValid(UFileDownloadHandler::Get())) {
		return;
	}

	FSlateApplication::Get().CancelDragDrop();

	UFileDownloadHandler::Get()->SetIsDragging(true);

	const FVector2D CurrentCursorPosition = FSlateApplication::Get().GetCursorPos();
	const FVector2D LastCursorPosition = FSlateApplication::Get().GetLastCursorPos();

	TSet<FKey> PressedMouseButtons;
	PressedMouseButtons.Add(EKeys::LeftMouseButton);

	FModifierKeysState ModifierKeyState;

	FPointerEvent FakePointerEvent(FSlateApplication::Get().GetUserIndexForMouse(), FSlateApplicationBase::CursorPointerIndex, CurrentCursorPosition, LastCursorPosition, PressedMouseButtons, EKeys::Invalid, 0, ModifierKeyState);

	FDragDropEvent DragDropEvent(FakePointerEvent, DragDropOperation);

	TSharedPtr<SWindow> OwnerWindow = FSlateApplication::Get().FindWidgetWindow(FRocket::Instance->BrowserDock.ToSharedRef());
	FSlateApplication::Get().ProcessDragEnterEvent(OwnerWindow.ToSharedRef(), DragDropEvent);
}

TSharedRef<FAssetDragDrop> FAssetDragDrop::New(FAssetData AssetDataArray, UActorFactory *ActorFactory, FString HTMLContent, FString Hash) {
	TSharedRef<FAssetDragDrop> Operation = MakeShareable(new FAssetDragDrop);
	Operation->Init({AssetDataArray}, TArray<FString>(), ActorFactory);
	Operation->HTMLContent = HTMLContent;
	Operation->Hash = Hash;
	Operation->Construct();
	return Operation;
}
