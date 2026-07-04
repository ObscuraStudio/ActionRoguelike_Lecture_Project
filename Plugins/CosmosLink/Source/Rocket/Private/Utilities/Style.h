// Copyright 2024 Leartes Studios. All Rights Reserved.

#pragma once

#include "Styling/SlateStyle.h"

// FStyle class is responsible for the styling of the Rocket tool.
class FStyle {
public:
	// Initializes the Rocket tool style.
	static void Initialize();

	// Shuts down the Rocket tool style.
	static void Shutdown();

	static FName GetToolStyleName() {
		return ToolStyleName;
	}

	// Returns the created Slate style set for the Rocket tool.
	static TSharedRef<FSlateStyleSet> GetCreatedToolSlateStyleSet() {
		return CreatedToolSlateStyleSet.ToSharedRef();
	}

private:
	// The name of the Rocket tool style.
	static FName ToolStyleName;

	// Creates a new Slate style set for the Rocket tool.
	static TSharedRef<FSlateStyleSet> CreateToolSlateStyleSet();

	// Holds the created Slate style set for the Rocket tool.
	static TSharedPtr<FSlateStyleSet> CreatedToolSlateStyleSet;

};
