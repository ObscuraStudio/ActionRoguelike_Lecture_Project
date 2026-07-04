// Copyright (c) 2025 Rafael Valoto/Publisher. All rights reserved.
// Created for: WindowsDualsense_ds5w - Plugin to support DualSense controller on Windows.
// Planned Release Year: 2025

#pragma once

#include "CoreMinimal.h"
#include "IHapticDevice.h"
#include "IInputDevice.h"
#include "Subsystems/AudioHapticsListener.h"


/**
 * Manages DualSense controllers, providing input and haptic feedback functionality.
 * This class interacts with platform-specific input device frameworks to handle
 * controllers, manage connections, update states, and process haptic feedback.
 */
class WINDOWSDUALSENSE_DS5W_API DeviceManager final : public IInputDevice, public IHapticDevice
{
public:
	virtual ~DeviceManager() override;

	/**
	 * Constructs a DeviceManager instance, initializing it with a message handler for input events
	 * and an audio haptics listener for processing audio-based haptic feedback.
	 *
	 * @param InMessageHandler The message handler responsible for capturing and processing input events.
	 */
	explicit DeviceManager(
		const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler
		);
	/**
	 * Called every frame to update Controller-State
	 * 
	 * @param DeltaTime Time
	 */
	virtual void Tick(float DeltaTime) override;
	/**
	 * Determines whether the specified controller supports force feedback functionality.
	 *
	 * @param ControllerId The unique identifier of the controller to check.
	 * @return true if the controller supports force feedback; false otherwise.
	 */
	virtual bool SupportsForceFeedback(int32 ControllerId) override
	{
		return true;
	}
	/**
	 * Sets the force feedback values for a specific controller.
	 * Updates the intensity of vibration or other force effects on the specified DualSense controller.
	 *
	 * @param ControllerId The identifier of the controller for which force feedback is being set.
	 * @param Values The force feedback values, including intensity and other parameters.
	 */
	virtual void SetChannelValues(int32 ControllerId, const FForceFeedbackValues& Values) override;
	/**
	 * Sets a specific property of the DualSense input device for the given controller.
	 * Allows configuration of properties such as light color and trigger resistance.
	 *
	 * @param ControllerId ID of the controller to apply the property to.
	 * @param Property A pointer to the input device property to set. This includes details like property name and value.
	 */
	virtual void SetDeviceProperty(int32 ControllerId, const FInputDeviceProperty* Property) override;
	/**
	 * Sets the light bar color on the specified DualSense controller.
	 * This method updates the LED output to match the given color.
	 *
	 * @param ControllerId The unique identifier for the controller to update.
	 * @param Color The desired color to be applied to the controller's light bar.
	 */
	virtual void SetLightColor(int32 ControllerId, FColor Color) override;
	/**
	 * Resets the light color of the specified DualSense controller to its default state.
	 * This function is a no-op if lazy loading mode is enabled or if the controller
	 * library instance cannot be retrieved for the given controller ID.
	 *
	 * @param ControllerId The ID of the controller whose light color is to be reset.
	 */
	virtual void ResetLightColor(int32 ControllerId) override;
	/**
	 * Executes a command in the context of the provided world.
	 * This function is typically used for handling console commands.
	 *
	 * @param InWorld The world context in which the command is executed.
	 * @param Cmd The command string to be executed.
	 * @param Ar The output device to log execution results or messages.
	 * @return True if the command was successfully executed, otherwise false.
	 */
	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override { return false; }
	/**
	 * Sets the haptic feedback values for a specific controller and hand.
	 * This function updates the haptic feedback values for the specified controller using the supplied values structure.
	 * The function does nothing if lazy loading is enabled or the library instance for the controller cannot be retrieved.
	 *
	 * @param ControllerId The unique identifier of the controller.
	 * @param Hand The hand for which to set the haptic feedback (e.g., left or right hand).
	 * @param Values The structure containing the haptic feedback values to apply.
	 */
	virtual void SetHapticFeedbackValues(int32 ControllerId, int32 Hand, const FHapticFeedbackValues& Values) override;
	/**
	 * Retrieves the range of haptic frequency supported by the DualSense controller.
	 *
	 * @param MinFrequency Reference to a float where the minimum frequency will be stored.
	 * @param MaxFrequency Reference to a float where the maximum frequency will be stored.
	 */
	virtual void GetHapticFrequencyRange(float& MinFrequency, float& MaxFrequency) const override {}
	/**
	 * Retrieves the scale factor applied to haptic amplitude for the device.
	 * Used to control the intensity of haptic feedback.
	 *
	 * @return The scale factor for haptic amplitude, with a default value of 1.0f.
	 */
	virtual float GetHapticAmplitudeScale() const override { return 1.0f; }
	/**
	 * Retrieves the haptic device interface implementation.
	 * This method allows access to the haptic device functionality provided by the DualSense controller.
	 *
	 * @return Pointer to the current instance as an IHapticDevice interface.
	 */
	virtual IHapticDevice* GetHapticDevice() override { return this; }
	/**
	 * Determines whether a gamepad is currently attached.
	 * This method always indicates that a DualSense gamepad is attached.
	 *
	 * @return True if a gamepad is attached; false otherwise.
	 */
	virtual bool IsGamepadAttached() const override { return true; }
	/**
	 * Handles user login state changes
	 * @param bLoggedIn Whether a User is logged in
	 * @param UserId Platform-specific user identifier
	 * @param UserIndex Index of the user
	 */
	void OnUserLoginChangedEvent(bool bLoggedIn, int32 UserId, int32 UserIndex) const;
	/**
	 * Sends controller input events to the appropriate systems for processing.
	 * This method is overridden to handle specific input events from the DualSense controller,
	 * ensuring they are relayed correctly within the application.
	 */
	virtual void SendControllerEvents() override;
	/**
	 * Sets the message handler for the application to process input events.
	 *
	 * @param InMessageHandler Reference to the shared generic application message handler used to handle input events.
	 */
	virtual void SetMessageHandler(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) override
	{
	}
	/**
	 * Sets the force feedback intensity for a specific channel on a particular controller.
	 * Used to deliver haptic feedback through the given channel.
	 *
	 * @param ControllerId The unique identifier of the controller to set the channel value for.
	 * @param ChannelType The type of the feedback channel to update (e.g., left motor, right motor).
	 * @param Value The intensity of the feedback typically ranging from 0.0 (no feedback) to 1.0 (maximum feedback).
	 */
	virtual void SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value) override
	{
	}

private:
	FInputDeviceId GetGamepadInterface(int32 ControllerId);
	/**
	 * Tracks the accumulated time or events between periodic polling operations.
	 * This variable is typically used to manage timing or frequency of polling processes.
	 */
	float PollAccumulator = 0.0f;
	/**
	 * Defines the interval, in seconds, between periodic polling operations.
	 * This variable determines how often certain tasks, such as device state checks
	 * or updates, are performed within the system.
	 */
	float PollInterval = 0.033f;
	/**
	 * Stores a mapping of connection states for devices, where the key represents
	 * a device ID (int32) and the value indicates whether a connection change
	 * has occurred (true or false).
	 * This data structure is used to track and update connection status dynamically.
	 */
	TMap<int32, bool> IsConnectionChange = TMap<int32, bool>();
	/**
	 * Handles application-level messages and events, facilitating communication
	 * between the application framework and platform-specific input systems.
	 * This variable is used to process input events, updates, and interactions
	 * within the application.
	 */
	const TSharedRef<FGenericApplicationMessageHandler>& MessageHandler;
	// /**
	//  * Represents a shared reference to the audio haptics listener, used for managing
	//  * audio-driven haptic feedback functionality. This reference allows reactive haptic
	//  * responses synchronized with audio events or signals.
	//  */
	// const TSharedRef<FAudioHapticsListener>& AudioHapticsListener;
};
