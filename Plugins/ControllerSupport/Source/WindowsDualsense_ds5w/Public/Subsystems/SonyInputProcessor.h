// Copyright (c) 2025 Rafael Valoto/Publisher. All rights reserved.
// Created for: WindowsDualsense_ds5w - Plugin to support DualSense controller on Windows.
// Planned Release Year: 2025

#pragma once

#include "CoreMinimal.h"
#include "Input/Events.h"
#include "InputCoreTypes.h"
#include "Framework/Application/IInputProcessor.h"

class FSonyInputProcessor : public IInputProcessor
{
public:
	virtual ~FSonyInputProcessor() override {}
	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override {};
	virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	virtual bool HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	virtual bool HandleMouseButtonDoubleClickEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	virtual bool HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent,
		const FPointerEvent* InGestureEvent) override;
	virtual bool HandleMotionDetectedEvent(FSlateApplication& SlateApp, const FMotionEvent& MotionEvent) override;
	virtual const TCHAR* GetDebugName() const override;


	/**
	 * Handles a key-down event and verifies if the input originates from a Sony controller.
	 *
	 * @param SlateApp The Slate application instance managing the input events.
	 * @param InKeyEvent The key-down event being processed.
	 * @return Returns true if the key-down event is associated with a Sony controller; otherwise, false.
	 */
	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;
	/**
	 * Handles a key-up event and determines if the event is originated from a Sony controller.
	 *
	 * @param SlateApp The Slate application instance managing the input events.
	 * @param InKeyEvent The key-up event being processed.
	 * @return Returns true if the key-up event is associated with a Sony controller; otherwise, false.
	 */
	virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;
	/**
	 * Processes an analog input event to determine if the input originates from a Sony controller.
	 *
	 * @param SlateApp The Slate application instance managing input events.
	 * @param InAnalogInputEvent The analog input event being processed.
	 * @return Returns true if the input event is associated with a Sony controller; otherwise, false.
	 */
	virtual bool HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent) override;
	/**
	 * Checks if the input device associated with the specified DeviceId is a Sony controller.
	 *
	 * @param DeviceId The unique identifier representing the input device to be checked.
	 * @return Returns true if the device is identified as a Sony controller; otherwise, false.
	 */
private:
	bool IsSonyController(const FInputDeviceId& DeviceId) const;
};
