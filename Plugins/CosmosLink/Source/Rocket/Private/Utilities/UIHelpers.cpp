// Copyright 2024 Leartes Studios. All Rights Reserved.

#include "UIHelpers.h"
#include "Editor.h"
#include "FileDownloadHandler.h"
#include "RocketModule.h"
#include "Selection.h"
#include "TempAssetHandler.h"
#include "ActorFactories/ActorFactoryAmbientSound.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "Sound/AmbientSound.h"
#include "Sound/SoundBase.h"
#include "Widgets/SWindow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"

TSharedPtr<SWindow> FUIHelpers::SoundSelectionWindow = nullptr;
TArray<TSharedPtr<FAssetData>> FUIHelpers::SharedAssetDataArray;
TSharedPtr<FAssetData> FUIHelpers::SelectedSoundAsset = nullptr;

void FUIHelpers::ShowSoundSelectionWindow(UFileDownloadHandler *Handler, const TArray<FAssetData> &AssetDataArray, const FString &InProductName, const FString &Hash) {
	if (SoundSelectionWindow.IsValid()) {
		SoundSelectionWindow->RequestDestroyWindow();
	}

	SharedAssetDataArray.Empty();
	for (const FAssetData &AssetData : AssetDataArray) {
		SharedAssetDataArray.Add(MakeShareable(new FAssetData(AssetData)));
	}

	SoundSelectionWindow = SNew(SWindow).Title(FText::FromString("Select Sound")).ClientSize(FVector2D(400, 600)).SupportsMinimize(false).SupportsMaximize(false).FocusWhenFirstShown(true);

	TSharedRef<SListView<TSharedPtr<FAssetData>>> SoundListView = SNew(SListView<TSharedPtr<FAssetData>>).ListItemsSource(&SharedAssetDataArray).OnGenerateRow_Lambda([](TSharedPtr<FAssetData> InItem, const TSharedRef<STableViewBase> &OwnerTable) -> TSharedRef<ITableRow> {
		return SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable).Padding(FMargin(5.0f)).Content()[SNew(STextBlock).Text(FText::FromName(InItem->AssetName)).ColorAndOpacity(FSlateColor(FLinearColor::White))];
	}).SelectionMode(ESelectionMode::Single).OnSelectionChanged_Lambda([](const TSharedPtr<FAssetData> &SelectedSound, ESelectInfo::Type SelectInfo) {
		SelectedSoundAsset = SelectedSound;
	});

	SoundSelectionWindow->SetContent(SNew(SBox).Padding(FMargin(10.0f))[SNew(SVerticalBox) + SVerticalBox::Slot().Padding(FMargin(10.0f)).VAlign(VAlign_Fill).HAlign(HAlign_Fill)[SoundListView] + SVerticalBox::Slot().Padding(FMargin(10.0f)).AutoHeight().HAlign(HAlign_Right)[SNew(SHorizontalBox) + SHorizontalBox::Slot().AutoWidth().Padding(FMargin(5.0f))[SNew(SButton).Text(FText::FromString("OK")).OnClicked_Lambda([Handler, InProductName, Hash]() -> FReply {
		if (SelectedSoundAsset.IsValid()) {
			OnImportButtonClicked(Handler, InProductName, Hash);
		}
		return FReply::Handled();
	})] + SHorizontalBox::Slot().AutoWidth().Padding(FMargin(5.0f))[SNew(SButton).Text(FText::FromString("Close")).OnClicked_Lambda([]() -> FReply {
		CloseSoundSelectionWindow();
		return FReply::Handled();
	})]]]);

	FSlateApplication::Get().AddModalWindow(SoundSelectionWindow.ToSharedRef(), FSlateApplication::Get().GetActiveTopLevelWindow());
}

void FUIHelpers::OnSoundSelected(const TSharedPtr<FAssetData> &SelectedSound, ESelectInfo::Type SelectInfo, UFileDownloadHandler *Handler, const FString &InProductName, const FString &Hash) {

	if (!SelectedSound.IsValid()) {
		UE_LOG(LogRocket, Error, TEXT("SelectedSound is not valid"));
		return;
	}

	USoundBase *SoundWave = Cast<USoundBase>(SelectedSound->GetAsset());
	if (!IsValid(SoundWave)) {
		UE_LOG(LogRocket, Error, TEXT("Failed to cast to USoundBase: %s"), *SelectedSound->PackageName.ToString());
		return;
	}

	TSharedPtr<FTempAssetHandler> *CurrentTempDataPtr = Handler->TempAssetData.Find(Hash);
	if (!CurrentTempDataPtr) {
		UE_LOG(LogRocket, Error, TEXT("Failed to find TempAssetHandler array for product: %s"), *InProductName);
		return;
	}

	for (auto CurrentTempActor : CurrentTempDataPtr->Get()->GetTempActors()) {

		if (!CurrentTempActor.IsValid()) {
			continue;
		}

		const UWorld *CurrentWorld = CurrentTempActor->GetWorld();
		if (!IsValid(CurrentWorld)) {
			continue;
		}

		if (UActorFactory *ActorFactory = NewObject<UActorFactoryAmbientSound>()) {
			AActor *NewActor = ActorFactory->CreateActor(SoundWave, CurrentWorld->GetCurrentLevel(), CurrentTempActor->GetTransform());
			if (AAmbientSound *NewAmbientSound = Cast<AAmbientSound>(NewActor)) {
				NewAmbientSound->GetAudioComponent()->SetSound(SoundWave);

				if (GEditor && GEditor->GetSelectedActors()->IsSelected(CurrentTempActor.Get())) {
					GEditor->SelectNone(false, true, false);
					GEditor->SelectActor(NewAmbientSound, true, true, false, true);
				}

				if (CurrentTempActor.IsValid()) {
					CurrentTempActor->Destroy();
				}

			} else {
				UE_LOG(LogRocket, Error, TEXT("Failed to create AmbientSound actor: %s"), *SelectedSound->PackageName.ToString());
			}
		} else {
			UE_LOG(LogRocket, Error, TEXT("Failed to create ActorFactoryAmbientSound"));
		}
	}

	CloseSoundSelectionWindow();
}

FReply FUIHelpers::OnImportButtonClicked(UFileDownloadHandler *Handler, const FString &InProductName, const FString &Hash) {
	if (SelectedSoundAsset.IsValid()) {
		OnSoundSelected(SelectedSoundAsset, ESelectInfo::Direct, Handler, InProductName, Hash);
		CloseSoundSelectionWindow();
	}
	return FReply::Handled();
}

void FUIHelpers::CloseSoundSelectionWindow() {
	if (SoundSelectionWindow.IsValid()) {
		SoundSelectionWindow->RequestDestroyWindow();
		SoundSelectionWindow = nullptr;
		SelectedSoundAsset = nullptr;
	}
}
