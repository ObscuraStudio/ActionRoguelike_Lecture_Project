// Copyright (c) 2025 Rafael Valoto/Publisher. All rights reserved.
// Created for: WindowsDualsense_ds5w - Plugin to support DualSense controller on Windows.
// Planned Release Year: 2025

#include "Subsystems/SonyInputProcessor.h"
#include "Core/DeviceRegistry.h"

bool FSonyInputProcessor::HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	const FInputDeviceId DeviceId = InKeyEvent.GetInputDeviceId();
	return IsSonyController(DeviceId);
}

bool FSonyInputProcessor::HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	const FInputDeviceId DeviceId = InKeyEvent.GetInputDeviceId();
	return IsSonyController(DeviceId);
}

bool FSonyInputProcessor::HandleAnalogInputEvent(FSlateApplication& SlateApp,
                                                 const FAnalogInputEvent& InAnalogInputEvent)
{
	const FInputDeviceId DeviceId = InAnalogInputEvent.GetInputDeviceId();
	return IsSonyController(DeviceId);
}

bool FSonyInputProcessor::IsSonyController(const FInputDeviceId& DeviceId) const
{
	ISonyGamepadInterface* Gamepad = FDeviceRegistry::Get()->GetLibraryInstance(DeviceId);
	if (Gamepad)
	{
		return true;
	}
	return false;
}

bool FSonyInputProcessor::HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	return IInputProcessor::HandleMouseMoveEvent(SlateApp, MouseEvent);
}

bool FSonyInputProcessor::HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	return IInputProcessor::HandleMouseButtonDownEvent(SlateApp, MouseEvent);
}

bool FSonyInputProcessor::HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	return IInputProcessor::HandleMouseButtonUpEvent(SlateApp, MouseEvent);
}

bool FSonyInputProcessor::HandleMouseButtonDoubleClickEvent(FSlateApplication& SlateApp,
	const FPointerEvent& MouseEvent)
{
	return IInputProcessor::HandleMouseButtonDoubleClickEvent(SlateApp, MouseEvent);
}

bool FSonyInputProcessor::HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent,
	const FPointerEvent* InGestureEvent)
{
	return IInputProcessor::HandleMouseWheelOrGestureEvent(SlateApp, InWheelEvent, InGestureEvent);
}

bool FSonyInputProcessor::HandleMotionDetectedEvent(FSlateApplication& SlateApp, const FMotionEvent& MotionEvent)
{
	return IInputProcessor::HandleMotionDetectedEvent(SlateApp, MotionEvent);
}

const TCHAR* FSonyInputProcessor::GetDebugName() const
{
	return IInputProcessor::GetDebugName();
}
