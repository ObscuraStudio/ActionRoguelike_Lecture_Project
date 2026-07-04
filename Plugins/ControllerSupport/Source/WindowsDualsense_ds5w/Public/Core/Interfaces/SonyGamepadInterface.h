// Copyright (c) 2025 Rafael Valoto/Publisher. All rights reserved.
// Created for: WindowsDualsense_ds5w - Plugin to support DualSense controller on Windows.
// Planned Release Year: 2025

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Core/Enums/EDeviceCommons.h"
#include "Core/Structs/DeviceContext.h"
#include "Core/Structs/DeviceSettings.h"
#include "InputCoreTypes.h"
#include "Misc/CoreDelegates.h"
#include "Runtime/ApplicationCore/Public/GenericPlatform/IInputInterface.h"
#include "Runtime/ApplicationCore/Public/GenericPlatform/GenericApplicationMessageHandler.h"
#include "SonyGamepadInterface.generated.h"

USTRUCT(BlueprintType)
struct FFeatureReport
{
	GENERATED_BODY()
};

/**
 * Interface for interacting with a Sony gamepad in a system.
 */
UINTERFACE()
class USonyGamepadInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface defining the operations and configurations for a Sony gamepad device.
 * This interface must be implemented to interact with the gamepad, handling tasks
 * such as connection management, input updates, and device settings.
 */
class WINDOWSDUALSENSE_DS5W_API ISonyGamepadInterface
{
	GENERATED_BODY()

public:
	/**
	 * Pure virtual function that checks the connection status of the gamepad.
	 *
	 * @return True if the gamepad is connected, false otherwise.
	 */
	virtual bool IsConnected() = 0;
	/**
	 * Retrieves the type of the device.
	 *
	 * @return The type of the device as an EDeviceType enumeration.
	 */
	virtual EDeviceType GetDeviceType() = 0;
	/**
	 * Retrieves the connection type of the device.
	 *
	 * @return The connection type of the device as an EDeviceConnection enumeration.
	 */
	virtual EDeviceConnection GetConnectionType() = 0;
	/**
	 * Initializes the gamepad library using the specified device context.
	 *
	 * This function is used to establish the initial connection and setup
	 * of the gamepad library for communication with the appropriate hardware
	 * or system interface. The `FDeviceContext` provides necessary information
	 * about the device such as its handle, connection type, and related settings.
	 *
	 * @param Context A reference to an `FDeviceContext` structure containing
	 *                the device's configuration and connection details.
	 * @return A boolean value indicating whether the library was successfully initialized.
	 *         - `true` if the initialization was successful.
	 *         - `false` if the initialization failed.
	 */
	virtual bool InitializeLibrary(const FDeviceContext& Context) = 0;
	/**
	 * Shuts down and cleans up resources related to the gamepad library.
	 *
	 * This method is responsible for properly deallocating and releasing any
	 * resources initialized or managed by the gamepad library. It should be
	 * called when the library is no longer needed to ensure no resource leaks
	 * or dangling references remain.
	 *
	 * Implementers must provide the logic to safely terminate any running
	 * processes, close device connections, and reset internal states as necessary.
	 *
	 * This function must be implemented by classes that inherit from the
	 * ISonyGamepadInterface interface.
	 */
	virtual void ShutdownLibrary() = 0;
	/**
	 * Sets the lightbar color and associated timing parameters on the gamepad.
	 *
	 * @param Color The color to set the lightbar to, represented as an FColor.
	 * @param BrithnessTime The time duration for setting the brightness effect. Defaults to 0.0f.
	 * @param ToggleTime The time interval for toggling the lightbar. Defaults to 0.0f.
	 */
	virtual void SetLightbar(FColor Color, float BrithnessTime = 0.0f, float ToggleTime = 0.0f) = 0;
	/**
	 * Sets the LED associated with the player on the Sony gamepad to a specified brightness level.
	 *
	 * @param Led The LED associated with a specific player, represented as an ELedPlayerEnum.
	 * @param Brightness The desired brightness level for the LED, represented as an ELedBrightnessEnum.
	 */
	virtual void SetPlayerLed(ELedPlayerEnum Led, ELedBrightnessEnum Brightness) = 0;
	/**
	 * Sets the state of the microphone LED on the gamepad.
	 *
	 * @param Led The desired state of the microphone LED, represented by ELedMicEnum.
	 */
	virtual void SetMicrophoneLed(ELedMicEnum Led) = 0;
	/**
	 * Sets the touch state for the device.
	 *
	 * @param bIsTouch A boolean indicating whether touch input is enabled (true) or disabled (false).
	 */
	virtual void EnableTouch(const bool bIsTouch) = 0;
	/**
	 * Resets the orientation of the gyroscope to its default state.
	 * Typically used to recalibrate the gyroscope sensor.
	 */
	virtual void ResetGyroOrientation() = 0;
	/**
	 * Enables the motion sensor functionality of the gamepad.
	 *
	 * @param bIsMotionSensor Specifies whether to enable the gyroscope (true) or accelerometer (false) as the motion sensor.
	 */
	virtual void EnableMotionSensor(bool bIsMotionSensor) = 0;
	/**
	 * Initiates the calibration process for the motion sensor on the gamepad.
	 *
	 * @param Duration The duration, in seconds, for which the calibration process should run.
	 * @param DeadZone The threshold value to be used for filtering out small movements.
	 */
	virtual void StartMotionSensorCalibration(float Duration, float DeadZone) = 0;
	/**
	 * Retrieves the current calibration status of the motion sensors.
	 *
	 * @param OutProgress A reference to a float where the current calibration progress will be stored.
	 *                    The value ranges from 0.0 (no progress) to 1.0 (fully calibrated).
	 * @return True if the calibration status was successfully retrieved, false otherwise.
	 */
	virtual bool GetMotionSensorCalibrationStatus(float& OutProgress) = 0;
	/**
	 * Retrieves the current battery level of the Sony gamepad.
	 *
	 * @return The battery level as a floating-point value, where the exact range
	 *         and representation are dependent on the implementation. Typically,
	 *         values may range between 0.0 (empty) and 1.0 (full).
	 */
	virtual float GetBattery() const = 0;
	/**
	 * Sets the vibration effect for the Sony gamepad.
	 *
	 * @param Values A reference to an FForceFeedbackValues struct containing the force feedback
	 *               intensity and duration for the vibration effect.
	 */
	virtual void SetVibration(const FForceFeedbackValues& Values) = 0;
	/**
	 * Provides access to the underlying mutable device context for low-level operations.
	 * Note: For advanced/console usage only. May return nullptr if not available.
	 */
	virtual FDeviceContext* GetMutableDeviceContext() = 0;
	/**
	 * Determines if the controller events are enabled for sending.
	 *
	 * @return True if controller events are being sent, false otherwise.
	 */
	virtual bool IsSendControllerEvents() = 0;
	/**
	 * Sets the controller events based on the specified change state.
	 *
	 * @param IsChanged A boolean value indicating whether the controller events have changed.
	 * @return True if the operation was successful, otherwise false.
	 */
	virtual void SetControllerEvents(const bool IsChanged) = 0;
	/**
	 * Pure virtual function that sends data or commands to the connected gamepad.
	 * This function must be implemented by any class inheriting this interface.
	 */
	virtual void SendOut() = 0;
	/**
	 * Stops all currently active operations or actions associated with the interface.
	 * This method must be implemented by any derived class to handle the termination
	 * of all ongoing processes.
	 */
	virtual void StopAll() = 0;
	/**
	 * Updates the input state for the Sony gamepad interface.
	 *
	 * @param InMessageHandler A shared reference to the message handler responsible for processing input events.
	 * @param UserId The platform-specific user identifier associated with the input.
	 * @param InputDeviceId The identifier for the input device being updated.
	 */
	virtual void UpdateInput(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler, const FPlatformUserId UserId, const FInputDeviceId InputDeviceId, float Delta) = 0;
};
