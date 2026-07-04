// Copyright (c) 2025 Rafael Valoto/Publisher. All rights reserved.
// Created for: WindowsDualsense_ds5w - Plugin to support DualSense controller on Windows.
// Planned Release Year: 2025

#pragma once

#include "CoreMinimal.h"
#include "OutputContext.generated.h"

/**
 * @class FAudioConfig
 *
 * @brief Represents the configuration settings for the audio system.
 *
 * The FAudioConfig class encapsulates various configuration parameters and
 * settings related to an audio system. It provides an interface to manage
 * audio configurations such as sample rate, channel count, and other audio
 * processing properties.
 *
 * This class is intended to be used as part of the audio system to define
 * or modify how audio is processed and output. It acts as a central place
 * for configuring audio-specific parameters.
 *
 * @note Ensure proper initialization of the configuration before using it
 *       with the audio processing pipeline.
 */
USTRUCT()
struct FAudioConfig
{
	GENERATED_BODY()

	/**
	 * @brief Represents a specific operational state or behavior mode of a system or application.
	 *
	 * Mode is typically used to define the way a system, process, or functionality operates
	 * under different conditions or configurations. The values it can hold are often predefined
	 * and may correspond to distinct states like manual, automatic, debug, or maintenance modes.
	 *
	 * The exact interpretation of the Mode variable depends on the context in which it is used.
	 *
	 * Usage of this variable may influence the overall behavior, decision-making,
	 * or output of the system it is a part of.
	 */
	uint8_t Mode = 0x05;
	/**
	 * Represents the audio volume level for the connected headset.
	 *
	 * The `HeadsetVolume` variable controls the output volume for audio transmitted
	 * to the headset connected to a DualSense device. It is an 8-bit unsigned integer,
	 * with a default value of `0x7C` (124 in decimal). The value can be adjusted
	 * programmatically to set the desired volume level.
	 *
	 * This variable is primarily used in device output configurations for transmitting
	 * audio-related settings to a DualSense controller.
	 */
	uint8_t HeadsetVolume = 0x7C;
	/**
	 * Represents the volume level of the speaker output for a DualSense device.
	 *
	 * The `SpeakerVolume` variable is utilized to manage and control the audio output level
	 * of the device's speaker. Its value can range from 0 to 255 (0x00 to 0xFF),
	 * where higher values indicate higher volume levels.
	 *
	 * Default Value: 0x7C (124 in decimal), which indicates an initial medium volume level.
	 *
	 * It is part of the `FAudioConfig` struct, which groups all the audio-related
	 * configurations for the Devices and is applied along with other features during
	 * HID report communication
	 *
	 * Usage:
	 * Used during DualSense Devices configuration and in HID report processing to
	 * adjust the speaker's output level.
	 */
	uint8_t SpeakerVolume = 0x7C;
	/**
	 * MicVolume is an 8-bit unsigned integer that represents the microphone volume level
	 * in an audio configuration. It is primarily used within the DualSense controller
	 * audio settings system to configure and control the microphone's output level.
	 *
	 * Default Value: 0x7C (124 in decimal), which is assumed to be a mid-to-high-level
	 * microphone volume setting.
	 *
	 * Usage Context:
	 * - The variable is part of the `FAudioConfig` struct, embedded within the
	 *   `FOutput` structure for managing controller output settings.
	 * - It is used during output buffer creation to send microphone volume data
	 *   to the hardware, particularly at buffer index `Output[6]` in a USB or Bluetooth context.
	 * - The value can be dynamically configured via the related API, such as the
	 *   `UDualSenseLibrary::Settings` method, using the provided settings structure.
	 */
	uint8_t MicVolume = 0x7C;
	/**
	 * @brief Represents the status of the microphone in DualSense audio settings.
	 *
	 * This variable is used to control and report the microphone status within
	 * the audio configuration settings of the DualSense device. Its value
	 * influences the output buffer and is transmitted to the device during
	 * communication.
	 *
	 * @details
	 * - Initial Value: 0x0
	 * - Encoding:
	 *   - 0x0: Microphone is off.
	 *   - 0x1: Microphone is on.
	 *
	 * Usage in Implementation:
	 * - This variable is part of the FAudioConfig structure and is modified
	 *   programmatically to reflect the desired microphone state.
	 * - Transmitted as part of the HID output data buffer during device communication.
	 * - Controlled via the settings API, influenced by user-defined configurations.
	 */
	uint8_t MicStatus = 0x0;
};

/**
 * Structure representing the configuration of features for a device.
 * This struct contains various settings related to vibration, rumble, and trigger behavior.
 */
USTRUCT()
struct FFeatureConfig
{
	GENERATED_BODY()

	/**
	 * Represents the mode of a feature in the FFeatureConfig struct.
	 *
	 * FeatureMode is used to configure a specific feature mode within the device.
	 * It is stored as an 8-bit unsigned integer, and its default value is 0xF7.
	 */
	uint8_t FeatureMode	  = 0xF7;
	/**
	 * @brief Represents the mode or pattern of vibration for a device.
	 *
	 * This variable is used to define or control the vibration behavior,
	 * such as intensity, duration, or specific patterns. It can be used
	 * in various contexts like notifications, alerts, or feedback mechanisms
	 * in hardware or software systems.
	 *
	 * The exact usage and values that VibrationMode can hold may depend
	 * on the underlying implementation and hardware capabilities.
	 *
	 * Typical applications include:
	 * - Haptic feedback
	 * - Alert notifications
	 * - Game controller vibration patterns
	 *
	 * It is recommended to ensure that the vibration mode is compatible with
	 * the target hardware and does not cause unintended behavior or excessive
	 * power consumption.
	 */
	uint8_t VibrationMode = 0xFC;
	/**
	 * @brief Adjusts and reduces the intensity of a soft rumble effect.
	 *
	 * The SoftRumbleReduce variable is used to control and diminish
	 * the intensity of a vibrational or rumbling feedback associated
	 * with a system or device. It provides a mechanism to finely tune
	 * or scale down the soft rumble effect to appropriate levels based
	 * on the desired application or user settings.
	 *
	 * Typical contexts for this variable include haptic feedback systems,
	 * gaming controllers, or any systems requiring adjustable rumble
	 * effects that necessitate a reduction in their intensity for user
	 * customization or system-specific requirements.
	 *
	 * This variable can be part of configurations to promote user
	 * comfort, power consumption management, or sensitivity adjustments.
	 */
	uint8_t SoftRumbleReduce = 0x01;
	/**
	 * Represents the softness level for the trigger component of a game controller.
	 *
	 * The value determines the sensitivity or resistance level of the trigger mechanism.
	 * It is represented as an 8-bit unsigned integer, typically loaded or updated based on
	 * device settings or user customization.
	 *
	 * Default value: 0x4 (hexadecimal representation).
	 * Usage:
	 * - The `TriggerSoftnessLevel` is employed to configure or control the physical feedback
	 *   of the controller's trigger mechanism.
	 * - Often combined with other features such as vibration or rumble effects.
	 */
	uint8_t TriggerSoftnessLevel = 0x01;
};

/**
 * @class FLightbar
 * @brief Represents a light bar with various properties and methods to control its behavior.
 *
 * The FLightbar class provides an interface for managing and interacting with a programmable light bar.
 * It includes functionality to set and get light patterns, manage brightness, and control other visual effects.
 * This class is typically used in applications where dynamic light effects are required.
 *
 * This class assumes that the underlying hardware is capable of supporting the requested behaviors.
 *
 * Key features include:
 * - Control of brightness settings.
 * - Configuration and retrieval of light patterns.
 * - Management of power state for the light bar.
 */
USTRUCT()
struct FLightbar
{
	GENERATED_BODY()

	/**
	 * Represents the variable R, typically used to store a specific value or parameter
	 * relevant to the context of the application or computation.
	 *
	 * The purpose and usage of R depend on the implementation details within the given context.
	 * It could be used to hold a numeric value, an object, or any other data type as required
	 * by the program's logic.
	 *
	 * Ensure that the value assigned to R aligns with the intended use case and data type
	 * requirements of the application.
	 *
	 * Proper initialization and handling of R are crucial for maintaining program stability
	 * and preventing potential runtime errors.
	 */
	unsigned char R = 0x0;
	/**
	 * @brief Reverses a given string in place.
	 *
	 * This function modifies the input string to reverse its characters. It swaps
	 * characters from the beginning and end of the string, moving towards the center.
	 *
	 * @param str The string to be reversed. Passed by reference.
	 */
	unsigned char G = 0x0;
	/**
	 * @brief Represents a variable used for storing data or intermediate values within a program.
	 *
	 * The variable B may serve various purposes depending on its context within the application.
	 * It could hold numeric, textual, or other data types, depending on the implementation.
	 *
	 * @note Ensure the type and purpose of B are clearly defined in the surrounding codebase to avoid misuse.
	 */
	unsigned char B = 0x0;
	/**
	 * @brief Represents a variable used within the program.
	 *
	 * The purpose and functionality of the variable `A` depend on its
	 * context and usage within the codebase. It can be assigned a value
	 * or modified during execution based on program requirements.
	 */
	unsigned char A = 0x0;
};

/**
 * Represents the player LED configuration for a device.
 */
USTRUCT()
struct FDualShockFlashLigthbar
{
	GENERATED_BODY()
	unsigned char Bright_Time = 0x0;
	unsigned char Toggle_Time = 0x0;
};

/**
 * Represents the player LED configuration for a device.
 */
USTRUCT()
struct FPlayerLed
{
	GENERATED_BODY()

	/**
	 * @brief Represents a LED (Light-Emitting Diode) control object.
	 *
	 * This variable is used to encapsulate the state and behavior of a LED. It can be used to
	 * manage the LED's on/off state, brightness, color, or other properties depending on the
	 * specific implementation.
	 *
	 * The exact behavior and attributes of the LED are dependent on the context in which the
	 * variable is implemented.
	 */
	unsigned char Led = 0x0;
	/**
	 * Represents the brightness level of the player's LED indicator on the controller.
	 *
	 * The value of this variable is an unsigned 8-bit integer (unsigned char) that can
	 * be adjusted to set the desired brightness level. The potential values range
	 * from 0x0 (minimum brightness or off) to the maximum value supported by the device.
	 *
	 * This variable is used in conjunction with player LED configurations
	 * and is passed to the output buffer to reflect the brightness level in real-time.
	 *
	 * Modifications to this variable impact the LED brightness visible on the device.
	 */
	unsigned char Brightness = 0x00;
};

/**
 * @class FRumbles
 *
 * @brief The FRumbles class is designed to manage and control rumble feedback for devices such as game controllers.
 *
 * This class provides functionalities to initialize, update, and manipulate rumble effects
 * as part of a feedback mechanism. It is intended to enhance user interaction by simulating
 * vibrations or force feedback based on specific events or triggers.
 *
 * The FRumbles class encapsulates all the necessary operations required to handle the lifecycle
 * of a rumble instance, providing methods for configuring rumble patterns, managing their intensity,
 * duration, and activation state.
 */
USTRUCT()
struct FRumbles
{
	GENERATED_BODY()

	/**
	 * @brief Represents the left-hand side position or value in a given context.
	 *
	 * The `Left` variable is often used to denote a position, direction, or value
	 * that is associated with or located at the left-side in a specific application
	 * or system. Its exact usage and meaning may vary depending on the context
	 * within which it is used.
	 *
	 * @note Ensure that the context or system in which `Left` is applied
	 * defines its behavior and associated interactions clearly.
	 */
	unsigned char Left = 0x0;
	/**
	 * @brief Represents the right direction or orientation in a spatial or logical context.
	 *
	 * The variable `Right` is typically used to signify the direction that is opposite to left.
	 * It can be applied in various domains such as geometry, navigation, or logical conditions
	 * where operations or processes involve orientation, movement, or side specification.
	 */
	unsigned char Right = 0x0;
};

/**
 * Represents a collection of strength-related configurations and parameters for haptic feedback.
 * This struct is primarily used to define zones, timings, and adjustable parameters
 * for haptic effects on controller triggers.
 */
USTRUCT()
struct FStrengths
{
	GENERATED_BODY()

	/**
	 * Represents the time period, expressed as an 8-bit unsigned integer.
	 * The `Period` variable is used to define the timing intervals for effects, such as in haptic triggers.
	 * It typically ranges from 0 to 255, with conversions from floating-point values done externally.
	 */
	uint8_t  Period = 0;
	/**
	 * @brief Represents a collection or list of zones that are active within a particular system, application,
	 *        or context of operation.
	 *
	 * The variable may be used to track, manage, or reference zones that are currently active, engaged, or
	 * enabled, depending on the specific implementation requirements.
	 *
	 * @note The exact structure or type of ActiveZones may vary and typically depends on the specific use case
	 *       or application logic.
	 */
	uint32_t ActiveZones = 0;
	/**
	 * Represents the combined data of time offsets and ratio parameters
	 * associated with trigger effects on a DualSense controller.
	 *
	 * The `TimeAndRatio` variable is encoded as a 32-bit unsigned integer. Its
	 * specific usage varies based on the mode of the trigger effect, storing
	 * data that controls timing and positional aspects of certain effects like
	 * galloping or similar dynamic responses.
	 *
	 * Use cases:
	 * - In Galloping mode (Mode 0x23), `TimeAndRatio` is used to encode timing
	 *   offsets for two specific trigger positions.
	 * - Acts as a bit-packed representation for trigger intervals or control data
	 *   within specific controller configurations.
	 */
	uint32_t TimeAndRatio = 0;
	/**
	 * A 64-bit variable representing the zones of strength in the trigger effect configuration.
	 * Each bit or group of bits within this variable may correspond to specific trigger zone parameters,
	 * allowing customization of haptic feedback for different trigger modes.
	 */
	uint64_t StrengthZones = 0;

	/**
	 *
	 */
	unsigned char Compose[10] = {0};
};

/**
 * @class FHapticTriggers
 * @brief Represents haptic feedback functionalities for triggers in a game context.
 *
 * The FHapticTriggers class is designed to control and simulate haptic feedback
 * for trigger inputs in supported devices. This allows developers to integrate
 * tactile sensations for enhanced user immersion and interactivity.
 *
 * This class provides a structure to define properties and features for haptic
 * trigger effects, such as intensity, vibration pattern, and other configuration
 * options that are specific to the haptic feedback mechanism.
 *
 * It is typically used in conjunction with input devices, such as game controllers
 * or other peripherals, to create a responsive and engaging user experience.
 */
USTRUCT()
struct FHapticTriggers
{
	GENERATED_BODY()
	/**
	 * @brief This function initializes the configuration settings required for the application.
	 *
	 * This function is responsible for setting up all the necessary initial parameters
	 * and configurations for the system. It ensures that the required environment
	 * and settings are prepared before further processing.
	 *
	 * @param configFilePath A string representing the path to the configuration file.
	 * @param isDebugMode A boolean indicating whether the debug mode is enabled.
	 *
	 * @return Returns true if the initialization is successful, otherwise false.
	 */
	unsigned char Mode = 0x0;
	/**
	 * Represents the frequency parameter for haptic feedback effects.
	 * This is used within haptic trigger feedback implementation.
	 * The value is typically converted and sent as part of trigger settings.
	 */
	unsigned char Frequency = 0x0;
	/**
	 * Represents the amplitude value for haptic triggers. This variable defines
	 * the intensity of the haptic feedback effect. The value is stored as an
	 * unsigned char, allowing for a range of 0 to 255.
	 */
	unsigned char Amplitude = 0x0;
	/**
	 * Represents the KeepEffect variable in the FHapticTriggers structure.
	 * This variable is used to determine whether a haptic effect should be
	 * maintained or stopped. It is an unsigned 8-bit character initialized
	 * to 0x0 by default.
	 */
	unsigned char KeepEffect = 0x0;
	/**
	 * @brief Represents the strength configuration used in haptic feedback effects for triggers.
	 *
	 * This variable is an instance of the FStrengths struct, which defines various attributes
	 * relating to haptic feedback, including timing, active zones, time and ratio characteristics,
	 * and strength zones. It is utilized in programs to configure and apply detailed haptic
	 * response patterns.
	 */
	FStrengths Strengths;
};

/**
 * @class FMicLight
 * @brief Represents a lighting mechanism for use in various simulations or rendering systems.
 *
 * The FMicLight class encapsulates properties and behavior for lights, providing features
 * such as intensity, color, and position control. It is designed for use in graphical
 * or physical simulations where light modeling is crucial.
 *
 * This class supports various types of lights (e.g., point light, directional light,
 * spotlights) and includes methods to manipulate their attributes for rendering or visual effects.
 *
 * Key functionalities include:
 * - Controlling light intensity and color.
 * - Managing light position and orientation.
 * - Emulating different types of light sources.
 * - Integration into larger scene management systems.
 */
USTRUCT()
struct FMicLight
{
	GENERATED_BODY()

	/**
	 * @brief Represents the mode of operation or state within a system or application.
	 *
	 * The `Mode` variable is used to indicate the current operational setting, behavior,
	 * or state. This may include configurations such as operational modes, states of
	 * functionality, or system-defined parameters. The specific meaning and values
	 * associated with `Mode` should be defined within the context where it is utilized.
	 *
	 * Use `Mode` to control or query the current operational context, enabling flexible
	 * behavior based on its value.
	 */
	unsigned char Mode = 0x0;
};

/**
 * @class FOutput
 * @brief The FOutput class is responsible for managing and handling output functionalities.
 *
 * This class provides methods and attributes that enable output operations, such as
 * processing, formatting, or directing the output to different destinations.
 * It may include support for various data types, customization of output format,
 * and other related functionalities required for output handling.
 *
 * Responsibilities of the FOutput class:
 * - Enable efficient output of data to designated targets.
 * - Support customizable output formatting and styles.
 * - Provide mechanisms for error handling or logging during output operations.
 *
 * Design considerations:
 * - Ensure flexibility and extensibility to adapt to various output destinations.
 * - Maintain compatibility with different output data formats if applicable.
 */
USTRUCT()
struct FOutputContext
{
	GENERATED_BODY()

	/**
	 * @brief Represents the Lightbar configuration for output operations.
	 *
	 * The Lightbar is used to control the RGB color output of the device. It is commonly used
	 * to visually convey device states or enhance user experience by changing colors.
	 * The R, G, B, and A components of the light color can be adjusted for customization.
	 */
	FLightbar Lightbar;
	/**
	 * @class FDualShockFlashLigthbar
	 *
	 * @brief Manages the lightbar functionality for a DualShock controller.
	 *
	 * The FDualShockFlashLigthbar class is designed to handle the control and
	 * customization of the lightbar on a DualShock controller. It provides
	 * functionality to define and implement light patterns, colors, and flashing
	 * behaviors suitable for various gaming or application scenarios.
	 *
	 * This class is intended to interact with the DualShock controller hardware,
	 * allowing dynamic updates to the lightbar as required by the application logic.
	 *
	 * @note Proper integration and hardware compatibility are required to ensure
	 *       the expected behavior of the lightbar.
	 */
	FDualShockFlashLigthbar FlashLigthbar;
	/**
	 * Represents the microphone light's configuration or mode
	 * used by the associated device. This structure is a component
	 * of the overall output configuration for the device and can
	 * control visual feedback through the microphone light.
	 *
	 * The `Mode` member variable determines how the microphone light behaves.
	 *
	 * Usage context:
	 * - Defined as part of output-specific settings in `FOutput`.
	 * - Utilized in HID report buffering, particularly to set the microphone light
	 *   mode when relaying device output data.
	 */
	FMicLight MicLight;
	/**
	 * Represents the configuration of the player's LED on a device.
	 *
	 * FPlayerLed is used to control and specify the state of the player's
	 * indicator LED on a controller, including its brightness and which LED is active.
	 * It is utilized within the FOutput structure and controller output protocols
	 * to define LED behavior based on player or system states.
	 */
	FPlayerLed PlayerLed;

	/**
	 * Represents the rumble settings for a DualSense controller.
	 * This struct contains two unsigned char variables, Left and Right,
	 * which determine the intensities for the left and right rumble motors respectively.
	 *
	 * Used within FOutput to define the vibration feedback for the controller.
	 * The values for Left and Right range from 0x00 (no vibration) to 0xFF (maximum vibration).
	 *
	 * The Rumbles variable is utilized in multiple functions across the DualSense API,
	 * managing rumble intensities during feedback events.
	 */
	FRumbles Rumbles;
	/**
	 * Represents the audio configuration for a DualSense device. This variable encapsulates
	 * settings related to audio levels and microphone status.
	 *
	 * The Audio variable is part of a hierarchical output structure, which includes other device
	 * settings such as rumbles, lights, and feature-specific configurations.
	 *
	 * This variable interacts directly with the device's output buffer during data transfer,
	 * ensuring that audio-related parameters such as volume and microphone state are configured correctly.
	 *
	 * Fields:
	 * - Mode: Defines the audio mode or configuration setting.
	 * - HeadsetVolume: Specifies the volume level for the headset.
	 * - SpeakerVolume: Specifies the volume level for the speaker.
	 * - MicVolume: Defines the volume level for the microphone.
	 * - MicStatus: Indicates the status of the microphone, such as enabled or disabled.
	 */
	FAudioConfig Audio;
	/**
	 * Represents configuration details for device features such as vibration, rumble, and trigger settings.
	 *
	 * The `Feature` variable is an instance of the FFeatureConfig structure, which contains the following:
	 *
	 * - `FeatureMode`: Defines the operational mode for features.
	 * - `VibrationMode`: Specifies the vibration mode for the device.
	 * - `SoftRumbleReduce`: Controls the reduction level for soft rumble effects.
	 * - `TriggerSoftnessLevel`: Determines the softness level for triggers.
	 *
	 * Used as part of the output configuration sent to the device, enabling customization and control
	 * over various aspects of the device's behavior.
	 */
	FFeatureConfig Feature;
	/**
	 * Represents the haptic feedback configuration for the left adaptive trigger
	 * of a DualSense controller. This variable is a part of the FOutput structure.
	 * It provides control over various trigger haptic effects such as vibration
	 * mode, amplitude, frequency, and active effect zones.
	 *
	 * The LeftTrigger variable interacts with various controller feedback systems
	 * for implementing precise and immersive trigger effects during gameplay.
	 *
	 * Key properties include:
	 * - Mode: Defines the type of haptic effect applied to the trigger.
	 * - Frequency: Sets the frequency of the vibration for fine-grained feedback.
	 * - Amplitude: Determines the strength of the haptic effect.
	 * - Strengths: Encapsulates zones and their corresponding strength levels.
	 *
	 */
	FHapticTriggers LeftTrigger;
	/**
	 * Represents the haptic feedback configuration for the right trigger of a DualSense controller.
	 *
	 * The `RightTrigger` variable is used to define various haptic trigger settings.
	 * It includes functionality to control the mode, frequency, amplitude, and effects
	 * of the right trigger, enhancing tactile feedback for the user.
	 *
	 * This variable is a part of the controller output structure and interacts with the
	 * hardware through buffered outputs.
	 *
	 * Associated with trigger feedback adjustments:
	 * - Mode: Defines the operational mode of the right trigger.
	 * - Strengths: Configures active and strength zones for fine-tuned trigger behavior.
	 *
	 * Used primarily in functions such As
	 * - SetHapticFeedbackValues(): To modify frequency based on specific feedback values.
	 * - SetTriggers(): To adjust the mode and strength zones for the right trigger.
	 *
	 * Additionally, this variable is integrated into the output process, where its
	 * values are buffered and transmitted to the device.
	 *
	 * This configuration allows seamless customization for immersive user interaction.
	 */
	FHapticTriggers RightTrigger;
};
