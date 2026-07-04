// Copyright (c) 2025 Rafael Valoto/Publisher. All rights reserved.
// Created for: WindowsDualsense_ds5w - Plugin to support DualSense controller on Windows.
// Planned Release Year: 2025

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Core/Enums/EDeviceCommons.h"
#include "Core/Enums/EDeviceConnection.h"
#if PLATFORM_WINDOWS
#include "Windows/WindowsApplication.h"
#endif
#include "SonyGamepadProxy.generated.h"


/**
 * Proxy class for interacting with Sony gamepad devices such as DualSense or DualShock controllers.
 * This class provides static methods to handle various functionalities, including device status,
 * connection type, battery levels, LED effects, motion sensor calibration, touch capabilities,
 * and deprecation of certain methods.
 */
UCLASS(Blueprintable, BlueprintType)
class WINDOWSDUALSENSE_DS5W_API USonyGamepadProxy : public UObject
{
	GENERATED_BODY()
	

public:
	/**
	 * Checks if the DualSense or DualShock device with the specified Controller ID is connected.
	 *
	 * @param ControllerId The ID of the controller to check for connectivity.
	 * @return True if the DualSense or DualShock  device is connected, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "SonyGamepad: Dualsense or DualShock Status")
	static bool DeviceIsConnected(int32 ControllerId);
	/**
	 * Retrieves the type of the connected device (e.g., DualSense, DualSense Edge, or DualShock 4)
	 * based on the specified Controller ID.
	 *
	 * @param ControllerId The ID of the connected controller whose type is to be determined.
	 * @return The type of the device as an EDeviceType enum value. Possible values include:
	 *         - DualSense
	 *         - DualSenseEdge
	 *         - DualShock4
	 */
	UFUNCTION(BlueprintCallable, Category = "SonyGamepad: Dualsense or DualShock Status")
	static EDeviceType GetDeviceType(int32 ControllerId);
	/**
	 * Retrieves the connection type of the DualSense or DualShock device with the specified Controller ID.
	 *
	 * @param ControllerId The ID of the controller whose connection type is being queried.
	 * @return The connection type of the device, which can be USB, Bluetooth, or Unrecognized.
	 */
	UFUNCTION(BlueprintCallable, Category = "SonyGamepad: Dualsense or DualShock Status")
	static EDeviceConnection GetConnectionType(int32 ControllerId);
	/**
	 * Retrieves the battery level of the DualSense or DualShock controller for the specified controller ID.
	 *
	 * This method uses the DualSense or DualShock library instance associated with the provided controller ID
	 * to fetch the battery level. If the library instance is not found, the method returns 0.0f.
	 *
	 * @param ControllerId The ID of the DualSense or DualShock controller to query.
	 * @return The battery level of the controller as a float. Returns 0.0f if the library instance cannot be retrieved.
	 */
	UFUNCTION(BlueprintCallable, Category = "SonyGamepad: Dualsense or DualShock Status")
	static float LevelBatteryDevice(int32 ControllerId);

	/**
	 * Updates the LED color effects on a DualSense controller using the specified color.
	 *
	 * @param ControllerId The identifier of the controller whose LED color will be updated.
	 * @param Color The color to set on the controller's LED.
	 */
	UFUNCTION(BlueprintCallable, Category = "SonyGamepad: Dualsense or DualShock Led Effects")
	static void LedColorEffects(
		int32 ControllerId,
		FColor Color,
		UPARAM(DisplayName = "(DualShock 4) LED brightness transition time min: 0.0f max: 2.5f", meta = (ClampMin = "0.0", ClampMax = "2.5", UIMin = "0.0", UIMax = "2.5", ToolTip = "(DualShock) LED brightness transition time, in seconds."))
		const float BrightnessTime = 0.0f,
		UPARAM(DisplayName = "(DualShock 4) Toggle transition time min: 0.0f max: 2.5f",  meta = (ClampMin = "0.0", ClampMax = "2.5", UIMin = "0.0", UIMax = "2.5", ToolTip = "(DualShock) Toggle transition time, in seconds."))
		const float ToogleTime = 0.0f
	);
	/**
	 * Controls the LED and microphone visual effects on a DualSense controller.
	 *
	 * @param ControllerId The ID of the DualSense controller to be affected.
	 * @param Value The desired LED and microphone effect to apply, represented as an ELedMicEnum value.
	 */
	UFUNCTION(BlueprintCallable, Category = "SonyGamepad: Dualsense or DualShock Led Effects")
	static void LedMicEffects(int32 ControllerId, ELedMicEnum Value);
	/**
	 * Initiates the motion sensor calibration process for the specified controller.
	 * The calibration adjusts the motion sensor sensitivity and dead zone settings.
	 *
	 * @param ControllerId The ID of the controller to be calibrated.
	 * @param Duration The duration of the calibration process in seconds.
	 * @param DeadZone The sensitivity threshold below which motion input will be ignored.
	 */
	UFUNCTION(BlueprintCallable, Category = "SonyGamepad|Motion Sensors")
	static void StartMotionSensorCalibration(
		int32 ControllerId,
		UPARAM(DisplayName = "Calibration Duration (Seconds)", meta = (ClampMin = "1.0", ClampMax = "10.0", UIMin = "1.0", UIMax = "10.0", 
			ToolTip = "The time in seconds to collect sensor data for calculating the stable center (baseline). Longer durations can provide a more accurate baseline."))
			float Duration = 2.0f,
			UPARAM(DisplayName = "Noise Deadzone Percentage", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0", 
				ToolTip = "A percentage (0.0 to 1.0) of the sensor noise range to ignore after calibration. A higher value creates a larger deadzone, filtering out more residual noise but potentially ignoring very subtle movements."))
				float DeadZone = 0.5f
			);

	UFUNCTION(BlueprintCallable, Category = "SonyGamepad|Motion Sensors")
	static void ResetGyroOrientation(int32 ControllerId);
	/**
	 * Retrieves the calibration status of the motion sensor for the specified controller.
	 *
	 * @param ControllerId The ID of the controller whose motion sensor calibration status is being queried.
	 * @param Progress A reference to a variable where the calibration progress will be stored, expressed as a percentage.
	 * @return True if the calibration process is in progress, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "SonyGamepad|Motion Sensors")
	static bool GetMotionSensorCalibrationStatus(int32 ControllerId, float& Progress);
	/**
	 * Enables or disables the touch functionality on a specified DualSense controller.
	 *
	 * @param ControllerId The identifier of the controller for which the touch functionality should be enabled or disabled.
	 * @param bEnableTouch A boolean indicating whether to enable (true) or disable (false) the touch functionality.
	 */
	UFUNCTION(BlueprintCallable, Category = "SonyGamepad: Dualsense or DualShock Touch")
	static void EnableTouch(int32 ControllerId, bool bEnableTouch);
	/**
	 * Enables or disables the gyroscope functionality for a specified DualSense controller.
	 *
	 * @param ControllerId The ID of the controller for which the gyroscope functionality is to be modified.
	 * @param bEnableGyroscope Set to true to enable the gyroscope, or false to disable it.
	 */
	UFUNCTION(BlueprintCallable, Category = "SonyGamepad|Motion Sensors")
	static void EnableGyroscopeValues(int32 ControllerId, bool bEnableGyroscope);
	/**
	 * Remaps the specified gamepad ID to a new user and updates the old user's settings accordingly.
	 *
	 * This method ensures that the controller is reassigned from the old user to the new user
	 * and notifies the input device mapper about the pairing change.
	 *
	 * @param GamepadId The ID of the gamepad to be reassigned.
	 * @param UserId The ID of the new user to whom the gamepad is being assigned.
	 * @param OldUser The ID of the previous user who was associated with the gamepad.
	 */
	UE_DEPRECATED(
		5.1, "Methods refactored and deprecated as of plugin version v1.2.1.")
	UFUNCTION(BlueprintCallable, Category = "SonyGamepad: Remap Device from User",
		meta=(DeprecatedFunction, DeprecationMessage="Use GamepadCoOp Plugin"))
	static void RemapControllerIdToUser(int32 GamepadId, int32 UserId, int32 OldUser) {}

	/**
	 * Attempts to reconnect a DualSense or DualShock controller based on the given controller ID.
	 * If the controller is successfully reconnected, the operation returns true;
	 * otherwise, it returns false.
	 *
	 * @param ControllerId The ID of the controller to reconnect.
	 * @return Returns true if the controller was successfully reconnected, false otherwise.
	 */
	UE_DEPRECATED(
		5.1, "Methods refactored and deprecated as of plugin version v1.2.10")
	UFUNCTION(BlueprintCallable, Category = "SonyGamepad: Dualsense or DualShock Status",
		meta=(DeprecatedFunction, DeprecationMessage="Use GamepadCoOp Plugin"))
	static bool DeviceReconnect(int32 ControllerId) { return true; }

	/**
	 * Disconnects the DualSense or DualShock device associated with the given Controller ID.
	 * This method removes the library instance associated with the specified controller.
	 *
	 * @param ControllerId The ID of the DualSense or DualShock or DualShock controller to be disconnected.
	 * @return true if the disconnection was initiated successfully.
	 */
	UE_DEPRECATED(
		5.1, "Methods refactored and deprecated as of plugin version v1.2.10")
	UFUNCTION(BlueprintCallable, Category = "SonyGamepad: Dualsense or DualShock Status",
		meta=(DeprecatedFunction, DeprecationMessage="SonyGamepad: Dualsense or DualShock Status"))
	static bool DeviceDisconnect(int32 ControllerId) { return true; };

	/**
	 * Enables or disables accelerometer values for the specified controller.
	 *
	 * This method allows toggling the accelerometer functionality for a given
	 * controller ID. If the DualSense instance for the specified controller ID
	 * is not available, the function will return without performing any actions.
	 *
	 * @param ControllerId The ID of the controller for which the accelerometer values will be enabled or disabled.
	 * @param bEnableAccelerometer A boolean value that determines whether to enable or disable accelerometer values.
	 */
	UE_DEPRECATED(
		5.1, "Methods refactored and deprecated as of plugin version v1.2.14")
	UFUNCTION(BlueprintCallable, Category = "SonyGamepad: Dualsense or DualShock Touch, Use EnableGyroscopeValues")
	static void EnableAccelerometerValues(int32 ControllerId, bool bEnableAccelerometer)
	{
		EnableGyroscopeValues(ControllerId, bEnableAccelerometer);
	}

protected:
	UFUNCTION()
	static FInputDeviceId GetGamepadInterface(int32 ControllerId);
	
};
