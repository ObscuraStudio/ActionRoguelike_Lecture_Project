// Copyright (c) 2025 Rafael Valoto/Publisher. All rights reserved.
// Created for: WindowsDualsense_ds5w - Plugin to support DualSense controller on Windows.
// Planned Release Year: 2025

#pragma once

#include "CoreMinimal.h"
#include "EDeviceCommons.generated.h"

#define BTN_FN1          0x10
#define BTN_FN2          0x20
#define BTN_PADDLE_LEFT  0x40
#define BTN_PADDLE_RIGHT 0x80

#define BTN_DPAD_UP 0x8
#define BTN_DPAD_DOWN 0x02
#define BTN_DPAD_LEFT 0x01
#define BTN_DPAD_RIGHT 0x04

#define BTN_CROSS 0x20
#define BTN_SQUARE 0x10
#define BTN_CIRCLE 0x40
#define BTN_TRIANGLE 0x80

#define BTN_START 0x20
#define BTN_SELECT 0x10

#define BTN_LEFT_STICK 0x40
#define BTN_RIGHT_STICK 0x80

#define BTN_LEFT_SHOLDER 0x01
#define BTN_RIGHT_SHOLDER 0x02

#define BTN_LEFT_TRIGGER 0x04
#define BTN_RIGHT_TRIGGER 0x08

#define BTN_PAD_BUTTON 0x02
#define BTN_MIC_BUTTON 0x04
#define BTN_PLAYSTATION_LOGO 0x01

#define PLAYER_LED_LEFT 0x01
#define PLAYER_LED_RIGHT 0x10
#define PLAYER_LED_MIDDLE 0x04

#define PLAYER_LED_MIDDLE_LEFT 0x02
#define PLAYER_LED_MIDDLE_RIGHT 0x08

/**
 * @brief Enum class representing various LED microphone states.
 *
 * This enum class is used to define the states for LED microphone controls on a compatible device.
 *
 * Enum values:
 * - MicOff: Represents the "Mic Off" state.
 * - MicOn: Represents the "Mic On" state.
 * - Pulse: Represents the "Pulse" state.
 */
UENUM(BlueprintType)
enum class ELedMicEnum : uint8
{
	MicOff = 0x0 UMETA(DisplayName = "Mic Off"),
	MicOn = 0x1 UMETA(DisplayName = "Mic On"),
	Pulse = 0x2 UMETA(DisplayName = "Pulse")
};

/**
 * @enum ELedPlayerEnum
 * Enumeration for controlling DualSense LED light player effects.
 */
UENUM(BlueprintType)
enum class ELedPlayerEnum : uint8
{
	Off = 0 UMETA(DisplayName = "Led Player Off"),
	One = PLAYER_LED_MIDDLE UMETA(DisplayName = "Player One"),
	Two = PLAYER_LED_MIDDLE_RIGHT | PLAYER_LED_MIDDLE_LEFT UMETA(DisplayName = "Player Two"),
	Three = PLAYER_LED_RIGHT | PLAYER_LED_MIDDLE | PLAYER_LED_LEFT UMETA(DisplayName = "Player Three"),
	All  = PLAYER_LED_RIGHT | PLAYER_LED_MIDDLE_RIGHT | PLAYER_LED_MIDDLE_LEFT | PLAYER_LED_LEFT UMETA(DisplayName = "Player all led")
};

/**
 * @brief Enum defining brightness levels for LED effects on the DualSense controller.
 *
 * This enumeration specifies different brightness levels that can be assigned
 * to LED effects. It is designed to work with Blueprint environments.
 */
UENUM(BlueprintType)
enum class ELedBrightnessEnum : uint8
{
	Low = 0x03 UMETA(DisplayName = "Brightness Low"),
	Medium = 0x02 UMETA(DisplayName = "Brightness Medium"),
	High = 0x00 UMETA(DisplayName = "Brightness High")
};

/**
 * @brief Enum class representing audio feature states for a DualSense controller.
 *
 * This enum class is utilized to manage the audio-related features of a DualSense controller,
 * such as enabling or disabling audio functionality.
 *
 * Enum values:
 * - On: Indicates that the audio is enabled.
 * - Off: Indicates that the audio is disabled.
 */
UENUM(BlueprintType)
enum class EDualSenseAudioFeatureReport : uint8
{
	On UMETA(DisplayName = "Audio On"),
	Off UMETA(DisplayName = "Audio Off")
};

/**
 * @brief Enum class representing the feature report options for a DualSense device.
 *
 * This enum class defines the various modes that can be used for configuring the DualSense device's
 * haptic feedback and vibration functionalities.
 *
 * Enum values:
 * - Off: Disables all vibration and haptic feedback.
 * - DefaultRumble: Activates the default vibration mode.
 * - HapticSoftRumble: Enables audio-based vibration for a softer haptic experience.
 */
UENUM(BlueprintType)
enum class EDualSenseDeviceFeatureReport : uint8
{
	Off = 0,
	DefaultRumble = 0xFF UMETA(DisplayName = "Default"),
	HapticSoftRumble = 0xFC UMETA(DisplayName = "Audio based vibration")
};

/**
 * @brief Enum class defining the softness levels for DualSense controller triggers.
 *
 * This enum specifies different levels of trigger softness for DualSense controllers, which can
 * be used to adjust the tactile feedback of the triggers based on the desired gaming experience.
 *
 * Enum values:
 * - Off: Disables trigger softness entirely.
 * - VeryRigid: Represents a very rigid trigger feedback.
 * - Rigid: Represents rigid trigger feedback.
 * - Medium: Represents a medium level of softness for the trigger.
 * - Soft: Represents a soft trigger feedback.
 * - VerySoft: Represents a very soft trigger feedback.
 */
UENUM(BlueprintType)
enum class EDualSenseTriggerSoftnessLevel : uint8
{
	Off = 0,
	VeryRigid = 1 UMETA(DisplayName = "Very Rigid"),
	Rigid = 2 UMETA(DisplayName = "Rigid"),
	Medium = 4 UMETA(DisplayName = "Medium"),
	Soft = 6 UMETA(DisplayName = "Soft"),
	VerySoft = 8 UMETA(DisplayName = "Very Soft")
};
