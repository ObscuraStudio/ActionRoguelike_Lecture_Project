// Copyright (c) 2025 Rafael Valoto/Publisher. All rights reserved.
// Created for: WindowsDualsense_ds5w - Plugin to support DualSense controller on Windows.
// Planned Release Year: 2025


#include "WindowsDualsense_ds5w/Public/WindowsDualsense_ds5w.h"

#if PLATFORM_LINUX || PLATFORM_MAC
#include "SDL.h"
#include "Subsystems/SonyInputProcessor.h"
#include "Framework/Application/SlateApplication.h"
#endif
#include "InputCoreTypes.h"
#include "Misc/Paths.h"
#include "DeviceManager.h"

#define LOCTEXT_NAMESPACE "FWindowsDualsense_ds5wModule"

void FWindowsDualsense_ds5wModule::StartupModule()
{
	IModularFeatures::Get().RegisterModularFeature(IInputDeviceModule::GetModularFeatureName(), this);
	RegisterCustomKeys();
#if PLATFORM_LINUX || PLATFORM_MAC
	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) != 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to initialize subsystems of SDL: %s"), UTF8_TO_TCHAR(SDL_GetError()));
	}

	if (FSlateApplication::IsInitialized())
	{
		SonyInputProcessor = MakeShared<FSonyInputProcessor>();
		FSlateApplication::Get().RegisterInputPreProcessor(SonyInputProcessor);
	}
#endif
	
}

void FWindowsDualsense_ds5wModule::ShutdownModule()
{
#if PLATFORM_LINUX || PLATFORM_MAC
	SDL_Quit();

	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().UnregisterInputPreProcessor(SonyInputProcessor);
	}
#endif
	
}

TSharedPtr<IInputDevice> FWindowsDualsense_ds5wModule::CreateInputDevice(
	const TSharedRef<FGenericApplicationMessageHandler>& InCustomMessageHandler)
{
	return MakeShareable(new DeviceManager(InCustomMessageHandler));
}

void FWindowsDualsense_ds5wModule::RegisterCustomKeys()
{
	const FKey Mic("PS_Mic");
	const FKey Menu("PS_Menu");
	const FKey Shared("PS_Share");
	const FKey TouchButtom("PS_TouchButtom");
	const FKey PlayStationButton("PS_Button");
	const FKey PS_PushLeftStick("PS_PushLeftStick");
	const FKey PS_PushRightStick("PS_PushRightStick");
	const FKey PS_FunctionL("PS_FunctionL");
	const FKey PS_FunctionR("PS_FunctionR");
	const FKey PS_PaddleL("PS_PaddleL");
	const FKey PS_PaddleR("PS_PaddleR");

	EKeys::AddKey(FKeyDetails(
		PS_FunctionL,
		FText::FromString("PlayStation Left Function Button"),
		FKeyDetails::GamepadKey
	));

	EKeys::AddKey(FKeyDetails(
		PS_FunctionR,
		FText::FromString("PlayStation Right Function Button"),
		FKeyDetails::GamepadKey
	));

	EKeys::AddKey(FKeyDetails(
		PS_PaddleL,
		FText::FromString("PlayStation Left Paddle"),
		FKeyDetails::GamepadKey
	));

	EKeys::AddKey(FKeyDetails(
		PS_PaddleR,
		FText::FromString("PlayStation Right Paddle"),
		FKeyDetails::GamepadKey
	));

	EKeys::AddKey(FKeyDetails(
		PS_PushLeftStick,
		FText::FromString("PlayStation Left Thumbstick Button"),
		FKeyDetails::GamepadKey
	));

	EKeys::AddKey(FKeyDetails(
		PS_PushRightStick,
		FText::FromString("PlayStation Right Thumbstick Button"),
		FKeyDetails::GamepadKey
	));

	EKeys::AddKey(FKeyDetails(
		Shared,
		FText::FromString("PlayStation Share"),
		FKeyDetails::GamepadKey
	));

	EKeys::AddKey(FKeyDetails(
		Menu,
		FText::FromString("PlayStation Menu"),
		FKeyDetails::GamepadKey
	));

	EKeys::AddKey(FKeyDetails(
		PlayStationButton,
		FText::FromString("PlayStation Button"),
		FKeyDetails::GamepadKey
	));

	EKeys::AddKey(FKeyDetails(
		Mic,
		FText::FromString("PlayStation Mic"),
		FKeyDetails::GamepadKey
	));

	EKeys::AddKey(FKeyDetails(
		TouchButtom,
		FText::FromString("PlayStation Touchpad Button"),
		FKeyDetails::GamepadKey
	));
}

IMPLEMENT_MODULE(FWindowsDualsense_ds5wModule, WindowsDualsense_ds5w)
