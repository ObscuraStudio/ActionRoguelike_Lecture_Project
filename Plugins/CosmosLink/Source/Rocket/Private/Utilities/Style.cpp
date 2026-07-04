// Copyright 2024 Leartes Studios. All Rights Reserved.

#include "Style.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateTypes.h"

FName FStyle::ToolStyleName = FName("RocketStyle");
TSharedPtr<FSlateStyleSet> FStyle::CreatedToolSlateStyleSet = nullptr;

void FStyle::Initialize() {
	if (!CreatedToolSlateStyleSet.IsValid()) {
		CreatedToolSlateStyleSet = CreateToolSlateStyleSet();
		FSlateStyleRegistry::RegisterSlateStyle(*CreatedToolSlateStyleSet);
	}
}

void FStyle::Shutdown() {
	if (CreatedToolSlateStyleSet.IsValid()) {
		FSlateStyleRegistry::UnRegisterSlateStyle(*CreatedToolSlateStyleSet);
		CreatedToolSlateStyleSet.Reset();
	}
}

TSharedRef<FSlateStyleSet> FStyle::CreateToolSlateStyleSet() {
	TSharedRef<FSlateStyleSet> CustomStyleSet = MakeShareable(new FSlateStyleSet(ToolStyleName));

	const FString IconDirectory = IPluginManager::Get().FindPlugin(TEXT("Rocket"))->GetBaseDir() / "Resources";

	CustomStyleSet->SetContentRoot(IconDirectory);

	const FVector2D Icon20x20(20.f, 20.f);

	CustomStyleSet->Set("Rocket.RocketLogo", new FSlateImageBrush(IconDirectory / "Rocket.png", Icon20x20));

	return CustomStyleSet;
}
