// Copyright (c) 2025 Rafael Valoto/Publisher. All rights reserved.
// Created for: WindowsDualsense_ds5w - Plugin to support DualSense controller on Windows.
// Planned Release Year: 2025

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Core/Interfaces/SonyGamepadInterface.h"
#include "Core/Structs/DualShockFeatureReport.h"
#include "Async/TaskGraphInterfaces.h"
#include "DualShockLibrary.generated.h"

/**
 * 
 */
UCLASS()
class WINDOWSDUALSENSE_DS5W_API UDualShockLibrary : public UObject, public ISonyGamepadInterface
{
	GENERATED_BODY()

public:
	// Expose nullptr for direct buffer access (not supported on DS4)
	virtual FDeviceContext* GetMutableDeviceContext() override { return nullptr; }
	/**
	 * @brief Configures device settings for a connected device.
	 *
	 * This method allows customization and application of configuration
	 * settings for a specific device by modifying its settings object.
	 * It is typically overridden to provide device-specific configuration
	 * logic.
	 *
	 * @param Settings A reference to a FDeviceSettings object that holds
	 * the device's configuration options to be adjusted or updated.
	 */
	virtual void Settings(const FDualShockFeatureReport& Settings);
	/**
	 * @brief Initializes the DualSense library with the specified device context.
	 *
	 * This method sets up the library with the provided device context, configuring
	 * the necessary states for the DualSense controller, such as enabling or disabling
	 * certain features. It ensures the library is prepared to handle input from a connected
	 * DualSense device.
	 *
	 * @param Context A reference to an FDeviceContext structure that contains information
	 * about the current device, such as the connection type and configuration.
	 *
	 * @return Returns true if the library was successfully initialized.
	 */
	virtual bool InitializeLibrary(const FDeviceContext& Context) override;
	/**
	 * @brief Shuts down the DualSense library and releases related resources.
	 *
	 * This method is responsible for halting the operation of the DualSense library,
	 * including resetting button states, clearing device contexts, and releasing
	 * system handles. It ensures that all resources utilized by the library are
	 * properly deallocated to prevent potential memory leaks or resource conflicts.
	 *
	 * @details The method resets the internal button states, zeroes any output data
	 * associated with the HID device context, closes the associated device handle,
	 * and frees any allocated HID device contexts via the DualSense HID Manager.
	 * Once completed, a log message is generated to confirm the successful shutdown.
	 */
	virtual void ShutdownLibrary() override;
	/**
	 * @brief Checks if a DualSense device is currently connected.
	 *
	 * This method determines whether a DualSense controller is actively connected
	 * to the system. It queries the underlying device context to assess the connection
	 * status and provides a boolean value indicating the result.
	 *
	 * @return True if a DualSense controller is connected; otherwise, false.
	 */
	virtual bool IsConnected() override;
	/**
	 * @brief Sends output data to the connected DualSense controller.
	 *
	 * This method is responsible for transmitting data to the DualSense
	 * controller through the HID device context. It ensures that the
	 * output buffering mechanism processes the data correctly before
	 * managing communication with the controller.
	 *
	 * @details The method first verifies the connection status of the HID
	 * device context. If the device is not connected, the operation is aborted.
	 * Otherwise, it delegates the responsibility of handling the output
	 * buffering to the appropriate manager, ensuring proper data flow to the device.
	 */
	virtual void SendOut() override;
	/**
	 * @brief Handles button input events for a DualSense controller.
	 *
	 * This function checks the current state of a button on a DualSense controller and triggers
	 * appropriate input events (button press or release) based on the change in its state.
	 *
	 * @param InMessageHandler The message handler responsible for dispatching input events.
	 * @param UserId The platform user ID associated with the controller.
	 * @param InputDeviceId The unique identifier for the DualSense input device.
	 * @param ButtonName The name of the button being checked.
	 * @param IsButtonPressed A boolean indicating the current pressed state of the button (true if pressed, false otherwise).
	 */
	virtual void CheckButtonInput(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler,
								  const FPlatformUserId UserId, const FInputDeviceId InputDeviceId,
								  const FName ButtonName, const bool IsButtonPressed);
	/**
	 * @brief Updates the input state for a DualSense device.
	 *
	 * This method processes input data received from a DualSense controller and updates
	 * the application's state via the provided message handler. It ensures that the input
	 * is correctly mapped and associated with the specified platform user and input device.
	 *
	 * @param InMessageHandler A shared reference to the application's message handler that processes input events.
	 * @param UserId The identifier for the platform user associated with the input device.
	 * @param InputDeviceId The unique identifier of the input device to be updated.
	 * @return A boolean value indicating whether the input update was successful.
	 */
	virtual void UpdateInput(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler,
							 const FPlatformUserId UserId, const FInputDeviceId InputDeviceId, float Delta) override;
	/**
	 * Retrieves the current battery level of the DualSense controller.
	 *
	 * @return The battery level as a floating-point value, representing the charge percentage or level.
	 */
	virtual float GetBattery() const override
	{
		return LevelBattery;
	}
	/**
	 * Sets the color of the lightbar on the Sony gamepad.
	 *
	 * @param Color The desired color for the lightbar, represented as an FColor object.
	 */
	virtual void SetLightbar(FColor Color, float BrithnessTime = 0.0f, float ToggleTime = 0.0f) override;
	/**
	 * Sets the LED associated with the player on the Sony gamepad to a specified brightness level.
	 *
	 * @param Led The LED associated with a specific player, represented as an ELedPlayerEnum.
	 * @param Brightness The desired brightness level for the LED, represented as an ELedBrightnessEnum.
	 */
	virtual void SetPlayerLed(ELedPlayerEnum Led, ELedBrightnessEnum Brightness) override;
	/**
	 * Sets the state of the microphone LED on the gamepad.
	 *
	 * @param Led The desired state of the microphone LED, represented by ELedMicEnum.
	 */
	virtual void SetMicrophoneLed(ELedMicEnum Led) override;
	/**
	 * Stops all currently active operations or actions associated with the interface.
	 * This method must be implemented by any derived class to handle the termination
	 * of all ongoing processes.
	 */
	virtual void StopAll() override;
	/**
	 * Sets the vibration effect for the Sony gamepad.
	 *
	 * @param Values A reference to an FForceFeedbackValues struct containing the force feedback
	 *               intensity and duration for the vibration effect.
	 */
	virtual void SetVibration(const FForceFeedbackValues& Values) override;
	/**
	 * @brief Retrieves the connection type of the device.
	 *
	 * This method provides information about the connection type of the device,
	 * such as whether it is connected via USB, Bluetooth, or another supported
	 * connection type. It overrides the base class implementation to return
	 * the specific connection type associated with the HID device context.
	 *
	 * @return The connection type of the device as an enumerated value of type EDeviceConnection.
	 */
	virtual EDeviceConnection GetConnectionType() override
	{
		return HIDDeviceContexts.ConnectionType;
	}
	/**
	 * @brief Retrieves the type of device associated with the current context.
	 *
	 * This method provides the specific type of device being used, as represented
	 * by the associated device type enumeration. The retrieved value can be utilized
	 * to determine the characteristics or capabilities of the device in use.
	 *
	 * @return The device type as an enumeration value of EDeviceType.
	 */
	virtual EDeviceType GetDeviceType() override
	{
		return HIDDeviceContexts.DeviceType;	
	}
	/**
	 * @brief Checks if controller events should be dispatched.
	 *
	 * Determines whether the system is configured to send controller
	 * input events, allowing input processing for connected controllers.
	 *
	 * @return A boolean value where true indicates that controller events
	 * should be sent, and false indicates they should not.
	 */
	virtual bool IsSendControllerEvents() override
	{
		return IsChange;
	}

	/**
	 * @brief Sets the state of controller event handling.
	 *
	 * This method updates the controller's event-handling state based on
	 * the provided input. It can be used to enable or disable the event
	 * handling mechanism dynamically.
	 *
	 * @param IsChanged A boolean value indicating whether the controller
	 * event state should be updated.
	 */
	virtual void SetControllerEvents(const bool IsChanged) override
	{
		IsChange = IsChanged;
	}
	/**
	 * Sets the touch state for the device.
	 *
	 * @param bIsTouch A boolean indicating whether touch input is enabled (true) or disabled (false).
	 */
	virtual void EnableTouch(const bool bIsTouch) override;
	/**
	 * Enables the motion sensor functionality of the gamepad.
	 *
	 * @param bIsMotionSensor Specifies whether to enable the gyroscope (true) or accelerometer (false) as the motion sensor.
	 */
	virtual void EnableMotionSensor(bool bIsMotionSensor) override;
	/**
	 * @brief Initiates the calibration process for the motion sensor.
	 *
	 * This method triggers the calibration sequence for the motion sensor
	 * to improve its accuracy and responsiveness. Calibration adjusts for
	 * external factors and ensures stable performance of the sensor.
	 *
	 * @param Duration The duration of the calibration process in seconds.
	 * @param DeadZone The threshold value to filter minor unintended movements
	 * during calibration.
	 */
	virtual void StartMotionSensorCalibration(float Duration, float DeadZone) override {}
	/**
	 * Retrieves the current calibration status of the motion sensors.
	 *
	 * @param OutProgress A reference to a float where the current calibration progress will be stored.
	 *                    The value ranges from 0.0 (no progress) to 1.0 (fully calibrated).
	 * @return True if the calibration status was successfully retrieved, false otherwise.
	 */
	virtual bool GetMotionSensorCalibrationStatus(float& OutProgress) override {return false;}
	/**
	 * @brief Resets the gyro orientation to its default alignment.
	 *
	 * This method reinitializes the gyroscope orientation, typically
	 * to recalibrate it or correct drift. It should be invoked when
	 * a new reference orientation is needed.
	 */
	virtual void ResetGyroOrientation() override {}
	/**
	 * Represents the unique identifier assigned to a specific DualSense controller.
	 *
	 * This variable is used to differentiate connected DualSense controllers, enabling the system
	 * to identify which controller instance corresponds to ongoing operations or events. The
	 * `ControllerID` is integral to updating input, haptic feedback, LED settings, and managing
	 * individual device states in multi-controller environments.
	 */
	int32 ControllerID;
	/**
	 * A map representing the states of various buttons on a controller.
	 *
	 * Each key in the map is a button name (FName), and its associated value is a boolean
	 * indicating whether the button is currently pressed (`true`) or not pressed (`false`).
	 *
	 * This variable is primarily used for tracking button input states and ensuring
	 * accurate representation of input events, such as detecting when a button's state
	 * changes from pressed to released or vice versa.
	 *
	 * The map is updated dynamically during controller runtime, including in functions
	 * like CheckButtonInput, which ensures real-time synchronization of input states.
	 * It is reset during library shutdown to clear all stored button states.
	 */
	TMap<const FName, bool> ButtonStates;
protected:
	/**
	 * @brief The PlatformInputDeviceMapper is responsible for mapping platform-specific
	 *        input device data to a standardized format used within the application.
	 *
	 * This variable is part of the input handling system and plays a crucial role in
	 * ensuring compatibility between different platform input devices and the application's
	 * input handling logic. It abstracts the differences in input device representations
	 * across operating systems or platforms, providing a unified interface for accessing
	 * input data.
	 *
	 * Typical use cases include mapping device identifiers, translating input codes, and
	 * adapting platform-specific input behavior to match application requirements.
	 *
	 * The PlatformInputDeviceMapper handles tasks such as:
	 * - Mapping raw input device identifiers to application-specific identifiers.
	 * - Normalizing input event data across diverse platforms and devices.
	 * - Facilitating cross-platform input device compatibility.
	 */
	static FGenericPlatformInputDeviceMapper PlatformInputDeviceMapper;
private:
	bool IsChange = false;
	/**
	 * @brief Represents the current battery level of a device.
	 *
	 * This variable stores the battery level, typically expressed as a
	 * floating-point value representing the percentage of charge remaining.
	 */
	float LevelBattery;
	/**
	 * @brief A variable that indicates whether touch functionality is enabled or disabled.
	 *
	 * This variable is used to toggle the touch input capability of the system or application.
	 * When set to true, touch input is enabled, allowing the system to respond to touch events.
	 * When set to false, touch input is disabled, and touch interactions are ignored.
	 */
	bool bEnableTouch;
	/**
	 * @variable EnableAccelerometerAndGyroscope
	 * @brief Flags the activation of accelerometer and gyroscope sensors in the system.
	 *
	 * This variable determines whether the accelerometer and gyroscope functionalities
	 * are enabled for the system. When set to true, data from these sensors will be collected
	 * and utilized, typically for motion detection or orientation tracking.
	 *
	 * @details This flag is often used in systems that require motion input for functionality,
	 * such as gaming controllers, virtual reality devices, or motion-sensing applications.
	 * Disabling this may reduce resource usage but will disable motion-based features.
	 */
	bool EnableAccelerometerAndGyroscope;
	/**
	 * @brief Represents the context of a Human Interface Device (HID) used by DualSense controllers.
	 *
	 * This variable holds the FDeviceContext structure, which encapsulates the necessary state and
	 * configuration details for managing the connection, communication, and interaction with a
	 * specific DualSense gamepad device. It is primarily used within the UDualSenseLibrary class for
	 * initialization, input handling, and managing device-specific settings.
	 */
	FDeviceContext HIDDeviceContexts;
	/**
	 * @brief Defines the sensitivity threshold for sensors to ignore minor inputs.
	 *
	 * This variable sets a minimum threshold value, below which input
	 * data from sensors are considered insignificant and are ignored.
	 * It is used to reduce noise and unintentional movements, ensuring
	 * stable and accurate sensor data processing.
	 *
	 * @variable SensorsDeadZone A float representing the dead zone limit for sensor input.
	 */
	float SensorsDeadZone = 0.3f;
	/**
	 * @brief Represents the threshold value to filter out unintended analog stick movements.
	 *
	 * AnalogDeadZone defines a small range of stick movement near the neutral
	 * position that is treated as no input. This helps in ignoring minor stick
	 * drift or unintentional movement input due to hardware sensitivity.
	 *
	 * The value is in the range [0.0, 1.0], where 0.3f means 30% of the
	 * full analog stick range is treated as a dead zone.
	 */
	float AnalogDeadZone = 0.3f;
};
