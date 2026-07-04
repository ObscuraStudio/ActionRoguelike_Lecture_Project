// Copyright (c) 2025 Rafael Valoto/Publisher. All rights reserved.
// Created for: WindowsDualsense_ds5w - Plugin to support DualSense controller on Windows.
// Planned Release Year: 2025

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "SonyGamepadProxy.h"
#include "Core/HapticsRegistry.h"
#include "Core/Enums/EDeviceCommons.h"
#include "Core/Structs/DualSenseFeatureReport.h"
#include "DualSenseProxy.generated.h"


/**
 * @brief Proxy class for PlayStation DualSense controller interactions and effects.
 *
 * Provides static functions to control various features and settings of a connected
 * PlayStation DualSense controller such as LED settings, vibration, trigger effects,
 * and connectivity status.
 */
UCLASS(Blueprintable, BlueprintType)
class WINDOWSDUALSENSE_DS5W_API UDualSenseProxy : public USonyGamepadProxy
{
	GENERATED_BODY()

public:
	/**
	 * Updates the settings for a DualSense controller using its feature report.
	 *
	 * @param ControllerId The ID of the controller to be updated.
	 * @param Settings The settings to be applied to the controller, encapsulated in an FDualSenseFeatureReport structure.
	 */
	UFUNCTION(BlueprintCallable, Category = "DualSense Settings")
	static void DeviceSettings(int32 ControllerId, FDualSenseFeatureReport Settings);
	/**
	 * @brief Registers a specific audio submix for a DualSense controller.
	 *
	 * Links a given sound submix to a connected PlayStation DualSense controller, identified by ControllerId.
	 * Enables audio haptics functionality to synchronize sound experiences with the controller's haptics capability.
	 *
	 * @param ControllerId The identifier for the connected DualSense controller.
	 * @param Submix The audio submix to be registered for haptic feedback on the controller.
	 */
	UFUNCTION(BlueprintCallable, Category = "DualSense|Audio", meta = (DisplayName = "Register Submix"))
	static void RegisterSubmixForDevice(int32 ControllerId, USoundSubmix* Submix);
	/**
	 * @brief Unregisters a submix listener associated with the specified DualSense controller device.
	 *
	 * This function removes the haptics listener for the specified controller ID, ensuring
	 * that any previously registered submix for audio or haptic feedback is no longer active.
	 *
	 * @param ControllerId The ID of the DualSense controller for which the submix listener will be unregistered.
	 */
	UFUNCTION(BlueprintCallable, Category = "DualSense|Audio", meta = (DisplayName = "Unregister Submix"))
	static void UnregisterSubmixForDevice(int32 ControllerId);
	/**
	 * @brief Activates an automatic gun effect on a specified DualSense controller.
	 *
	 * Simulates the behavior of an automatic firearm by adjusting trigger tension
	 * dynamically based on the provided strength parameters, hand specification,
	 * and other effect controls.
	 *
	 * @param ControllerId The ID of the controller to apply the effect on.
	 * @param BeginStrength The initial strength of the trigger effect. Ranges from 0 to 8.
	 * @param MiddleStrength The middle strength level of the trigger effect. Ranges from 0 to 8.
	 * @param EndStrength The final strength of the trigger effect. Ranges from 0 to 8.
	 * @param Hand The hand (left or right) to which the effect is applied.
	 * @param KeepEffect If true, maintains the effect even after the input stops; otherwise, stops once input ceases.
	 * @param Frequency The rate or frequency of the automatic gun effect.
	 */
	UFUNCTION(BlueprintCallable, Category = "DualSense Effects")
	static void AutomaticGun(
		int32 ControllerId,
		UPARAM(DisplayName = "Begin Strength min: 0 max: 8", meta = (ClampMin = "0", ClampMax = "8", UIMin = "0", UIMax = "8"))
		int32 BeginStrength,
		UPARAM(DisplayName = "Middle Strength min: 0 max: 8", meta = (ClampMin = "0", ClampMax = "8", UIMin = "0", UIMax = "8"))
		int32 MiddleStrength,
		UPARAM(DisplayName = "End Strength min: 0 max: 8", meta = (ClampMin = "0", ClampMax = "8", UIMin = "0", UIMax = "8"))
		int32 EndStrength,
		EControllerHand Hand,
		bool KeepEffect,
		float Frequency = 5.0f
	);

	/**
	 * @brief Sets the GameCube trigger effect for the specified controller and hand.
	 *
	 * This method applies the preconfigured GameCube trigger effect to a connected DualSense
	 * controller using the specified controller ID and hand designation.
	 *
	 * @param ControllerId The identifier for the DualSense controller to target.
	 * @param Hand Specifies which hand (left or right) the effect should be applied to.
	 */
	UFUNCTION (BlueprintCallable, Category = "DualSense Effects")
	static void GameCube(
		int32 ControllerId,
		
		EControllerHand Hand
	);

	/**
	 * @brief Configures custom trigger effects for a PlayStation DualSense controller.
	 *
	 * This method allows customization of the trigger input by defining a sequence of hexadecimal
	 * byte strings. It communicates with the controller to apply the specified trigger effect patterns.
	 *
	 * @param ControllerId The ID of the controller to configure.
	 * @param Hand Specifies which hand (left or right) the controller is associated with.
	 * @param HexBytes An array of hexadecimal byte strings representing the desired trigger configuration.
	 *                 Maximum of 10 byte strings is permitted.
	 */
	UFUNCTION (BlueprintCallable, Category = "DualSense Effects")
	static void CustomTrigger(
		int32 ControllerId,
		EControllerHand Hand,
		const TArray<FString>& HexBytes
	);

	/**
	 * Sets haptic feedback for a DualSense controller.
	 *
	 * @param ControllerId The ID of the controller for which feedback is to be set.
	 * @param BeginStrength The strength of the feedback at the beginning.
	 * @param MiddleStrength The strength of the feedback in the middle.
	 * @param EndStrength The strength of the feedback at the end.
	 * @param Hand The hand (left or right) associated with the controller.
	 */
	UFUNCTION(BlueprintCallable, Category = "DualSense Effects")
	static void SetFeedback(
		int32 ControllerId,
		UPARAM(DisplayName = "Begin Strength min: 0 max: 8", meta = (ClampMin = "0", ClampMax = "8", UIMin = "0", UIMax = "8"))
		int32 BeginStrength,
		UPARAM(DisplayName = "Middle Strength min: 0 max: 8", meta = (ClampMin = "0", ClampMax = "8", UIMin = "0", UIMax = "8"))
		int32 MiddleStrength,
		UPARAM(DisplayName = "End Strength min: 0 max: 8", meta = (ClampMin = "0", ClampMax = "8", UIMin = "0", UIMax = "8"))
		int32 EndStrength,
		
		EControllerHand Hand
	);

	/**
	 * Applies a resistance effect to the trigger of a PlayStation DualSense controller.
	 *
	 * This method configures the specified trigger to provide resistance feedback based on
	 * the given start position, end position, and strength parameters. The effect is applied
	 * to the controller defined by the ControllerId and the trigger specified using the Hand parameter.
	 *
	 * @param ControllerId The identifier of the controller to apply the resistance effect.
	 * @param StartPosition The starting position of the resistance zone within the trigger's range. Value must be between 0 and 8.
	 * @param EndPosition The ending position of the resistance zone within the trigger's range. Value must be between 0 and 8.
	 * @param Strength The strength of the resistance effect. Value must be between 0 and 8, with higher values indicating stronger resistance.
	 * @param Hand The trigger to apply the effect to, specified as EControllerHand (e.g., Left or Right).
	 */
	UFUNCTION(BlueprintCallable, Category = "DualSense Effects")
	static void Resistance(
		int32 ControllerId,
		UPARAM(DisplayName = "Start Position min: 0 max: 8", meta = (ClampMin = "0", ClampMax = "8", UIMin = "0", UIMax = "8"))
		int32 StartPosition,
		UPARAM(DisplayName = "End Position min: 0 max: 8", meta = (ClampMin = "0", ClampMax = "8", UIMin = "0", UIMax = "8"))
		int32 EndPosition,
		UPARAM(DisplayName = "Strength min: 0 max: 8", meta = (ClampMin = "0", ClampMax = "8", UIMin = "0", UIMax = "8"))
		int32 Strength,
		
		EControllerHand Hand
	);

	/**
	 * Applies a continuous resistance effect on the adaptive trigger of a DualSense controller.
	 *
	 * @param ControllerId The ID of the controller to apply the effect to.
	 * @param StartPosition The starting position of the resistance effect. Valid range determined by internal validation.
	 * @param Strength The intensity of the resistance effect. Valid range determined by internal validation.
	 * @param Hand The controller hand (e.g., left or right) to which the resistance effect applies.
	 */
	UFUNCTION(BlueprintCallable, Category = "DualSense Effects")
	static void ContinuousResistance(
		int32 ControllerId,
		UPARAM(DisplayName = "Start Position min: 0 max: 8", meta = (ClampMin = "0", ClampMax = "8", UIMin = "0", UIMax = "8"))
		int32 StartPosition,
		UPARAM(DisplayName = "Strength min: 0 max: 8", meta = (ClampMin = "0", ClampMax = "8", UIMin = "0", UIMax = "8"))
		int32 Strength,
		
		EControllerHand Hand
	);

	/**
	 * Configures the bow effect on a DualSense controller.
	 *
	 * @param ControllerId The ID of the controller to configure the effect on.
	 * @param StartPosition The starting position of the trigger effect.
	 * @param EndPosition The ending position of the trigger effect.
	 * @param BeginStrength The strength of the trigger's resistance at the starting position.
	 * @param EndStrength The strength of the trigger's resistance at the ending position.
	 * @param Hand The hand (left or right) where the effect will be applied.
	 */
	UFUNCTION(BlueprintCallable, Category = "DualSense Effects")
	static void Bow(
		int32 ControllerId,
		UPARAM(meta = (ClampMin = "0", ClampMax = "8", UIMin = "0", UIMax = "8"))
		int32 StartPosition,
		UPARAM(meta = (ClampMin = "0", ClampMax = "8", UIMin = "0", UIMax = "8"))
		int32 EndPosition,
		UPARAM(meta = (ClampMin = "0", ClampMax = "8", UIMin = "0", UIMax = "8"))
		int32 BeginStrength,
		UPARAM(meta = (ClampMin = "0", ClampMax = "8", UIMin = "0", UIMax = "8"))
		int32 EndStrength,
		
		EControllerHand Hand
	);


	/**
	 * @brief Triggers a galloping vibration effect on a DualSense controller.
	 *
	 * This function sets up and activates a galloping vibration pattern by defining
	 * the movement positions, associated vibration intensity, frequency, and the hand
	 * where the effect plays.
	 *
	 * @param ControllerId The ID of the controller to apply the effect.
	 * @param StartPosition The initial position of the galloping effect.
	 * @param EndPosition The final position of the galloping effect.
	 * @param FirstFoot The intensity for the first "foot" step in the galloping effect.
	 * @param SecondFoot The intensity for the second "foot" step in the galloping effect.
	 * @param Frequency The frequency at which the galloping effect repeats.
	 * @param Hand Specifies whether the effect is applied to the left or right hand.
	 */
	UFUNCTION(BlueprintCallable, Category = "DualSense Effects")
	static void Galloping(
		int32 ControllerId,
		UPARAM(meta = (ClampMin = "0", ClampMax = "8", UIMin = "0", UIMax = "8"))
		int32 StartPosition,
		UPARAM(meta = (ClampMin = "0", ClampMax = "9", UIMin = "0", UIMax = "9"))
		int32 EndPosition,
		UPARAM(DisplayName = "First Foot min: 2 max: 8", meta = (ClampMin = "0", ClampMax = "8", UIMin = "0", UIMax = "8"))
		int32 FirstFoot,
		UPARAM(DisplayName = "Second Foot min: (Greater FirstFoot) max: 9", meta = (ClampMin = "0", ClampMax = "9", UIMin = "0", UIMax = "9"))
		int32 SecondFoot,
		UPARAM(DisplayName = "Frequency Example: 5.0", meta = (ClampMin = "0.0", ClampMax = "40.0", UIMin = "0.0", UIMax = "40.0"))
		float Frequency,
		
		EControllerHand Hand
	);

	/**
	 * Configures and applies machine-like haptic effects to the DualSense controller.
	 *
	 * @param ControllerId The ID representing the target controller to which effects will be applied.
	 * @param StartPosition The starting position where the haptic effect begins on the controller.
	 * @param EndPosition The ending position where the haptic effect finishes on the controller.
	 * @param FirstFoot The position of the first foot in the haptic cycling effect.
	 * @param LasFoot The position of the last foot in the haptic cycling effect.
	 * @param Frequency The frequency at which the haptic effect oscillates.
	 * @param Period The period of the haptic cycle in seconds.
	 * @param Hand Specifies which hand the effect is directed towards (left or right).
	 */
	UFUNCTION(BlueprintCallable, Category = "DualSense Effects")
	static void Machine(
		int32 ControllerId,
		UPARAM(meta = (ClampMin = "0", ClampMax = "8", UIMin = "0", UIMax = "8"))
		int32 StartPosition,
		UPARAM(meta = (ClampMin = "0", ClampMax = "8", UIMin = "0", UIMax = "8"))
		int32 EndPosition,
		UPARAM(meta = (ClampMin = "0", ClampMax = "8", UIMin = "0", UIMax = "8"))
		int32 FirstFoot,
		UPARAM(meta = (ClampMin = "0", ClampMax = "8", UIMin = "0", UIMax = "8"))
		int32 LasFoot,
		UPARAM(meta = (ClampMin = "0.015", ClampMax = "1.0", UIMin = "0.01", UIMax = "1.0"))
		float Frequency,
		UPARAM(meta = (ClampMin = "0.015", ClampMax = "1.0", UIMin = "0.01", UIMax = "1.0"))
		float Period,
		
		EControllerHand Hand
	);

	/**
	 * Configures a weapon effect on the DualSense controller using specified parameters.
	 *
	 * @param ControllerId The identifier for the target DualSense controller.
	 * @param StartPosition The starting position of the effect in the trigger. The value should be validated and within the range of allowed positions.
	 * @param EndPosition The ending position of the effect in the trigger. The value should be validated and within the range of allowed positions.
	 * @param Strength The strength of the weapon effect. The value should be validated and within the range of allowed strengths.
	 * @param Hand Specifies which controller hand (left or right) should be affected by the weapon effect.
	 */
	UFUNCTION(BlueprintCallable, Category = "DualSense Effects")
	static void Weapon(
		int32 ControllerId,
		UPARAM(DisplayName = "Start Position min: 2", meta = (ClampMin = "0", ClampMax = "8", UIMin = "0", UIMax = "8"))
		int32 StartPosition,
		UPARAM(DisplayName = "End Position max: 7", meta = (ClampMin = "0", ClampMax = "8", UIMin = "0", UIMax = "8"))
		int32 EndPosition,
		UPARAM(DisplayName = "Strength max: 8", meta = (ClampMin = "0", ClampMax = "8", UIMin = "0", UIMax = "8"))
		int32 Strength,
		
		EControllerHand Hand
	);

	/**
	 * Controls the LED player light effects on the DualSense controller.
	 *
	 * @param ControllerId The identifier for the target controller.
	 * @param Value The LED pattern enum specifying the LED configuration for the player indicator (e.g., Off, Player One, Player Two).
	 * @param Brightness The brightness level of the LED lights specified by an enum (e.g., Low, Medium, High).
	 */
	UFUNCTION(BlueprintCallable, Category = "DualSense Led Effects")
	static void LedPlayerEffects(int32 ControllerId, ELedPlayerEnum Value, ELedBrightnessEnum Brightness);

	/**
	 * Disables the resistance effect for the specified controller and hand.
	 *
	 * @param ControllerId The unique identifier for the DualSense controller.
	 * @param Hand The hand (left or right) associated with the effect to disable resistance for.
	 */
	UFUNCTION(BlueprintCallable, Category = "DualSense Reset Effects")
	static void NoResistance(int32 ControllerId,
	
		EControllerHand Hand);

	/**
	 * Stops the trigger effect on a specific controller for the specified hand.
	 *
	 * @param ControllerId The unique identifier of the controller for which the trigger effect should be stopped.
	 * @param HandStop Specifies which hand's trigger effect (left or right) should be stopped.
	 */
	UFUNCTION(BlueprintCallable, Category = "DualSense Reset Effects")
	static void StopTriggerEffect(int32 ControllerId,
	
		EControllerHand HandStop);

	/**
	 * Stops all trigger effects currently active for the specified DualSense controller.
	 *
	 * @param ControllerId The ID of the DualSense controller for which to stop all trigger effects.
	 */
	UFUNCTION(BlueprintCallable, Category = "DualSense Reset Effects")
	static void StopAllTriggersEffects(int32 ControllerId);

	/**
	 * Resets all haptic feedback effects for the specified DualSense controller.
	 *
	 * This method stops all active effects currently running on the specified DualSense controller.
	 *
	 * @param ControllerId The identifier of the controller whose effects are to be reset.
	 */
	UFUNCTION(BlueprintCallable, Category = "DualSense Reset Effects")
	static void ResetEffects(int32 ControllerId);


	/**
	 * Deprecated method for enabling or disabling touch functionality on a DualSense controller.
	 * This method has been replaced by EnableTouch and is retained for backward compatibility.
	 *
	 * @param ControllerId The ID of the controller on which the touch functionality is to be enabled or disabled.
	 * @param bEnableTouch A boolean value indicating whether to enable (true) or disable (false) the touch functionality.
	 */
	UE_DEPRECATED(
		5.1, "Methods refactored and deprecated as of plugin version v1.2.1. Use EnableTouch instead of EnableTouch1.")
	UFUNCTION(BlueprintCallable, Category="DualSense Touch Pad",
		meta=(DeprecatedFunction, DeprecationMessage="Use EnableTouch"))
	static void EnableTouch1(int32 ControllerId, bool bEnableTouch)
	{
		EnableTouch(ControllerId, bEnableTouch);
	}

	/**
	 * Sets a trigger haptic feedback effect for the DualSense controller. Deprecated as of a plugin version v1.2.1. Use AutomaticGun instead.
	 *
	 * @param ControllerId The ID of the controller to apply the effect to.
	 * @param StartPosition The starting position of the trigger effect (deprecated, not utilized in the replacement method AutomaticGun).
	 * @param BeginStrength The strength of the feedback at the beginning of the trigger press.
	 * @param MiddleStrength The strength of the feedback at the middle of the trigger press.
	 * @param EndStrength The strength of the feedback at the end of the trigger press.
	 * @param Hand The hand (left or right) of the controller to which the effect will be applied.
	 * @param KeepEffect Whether the effect should persist after the initial application.
	 */
	UE_DEPRECATED(
		5.1, "Methods refactored and deprecated as of plugin version v1.2.1. Use AutomaticGun instead of SetTriggerHapticFeedbackEffect.")
	UFUNCTION(BlueprintCallable, Category="DualSense Effects",
		meta=(DeprecatedFunction, DeprecationMessage="Use AutomaticGun"))
	static void SetTriggerHapticFeedbackEffect(
		int32 ControllerId,
		int32 StartPosition,
		int32 BeginStrength,
		int32 MiddleStrength,
		int32 EndStrength,
		
		EControllerHand Hand, bool KeepEffect
	)
	{
		AutomaticGun(ControllerId, BeginStrength, MiddleStrength, EndStrength, Hand, KeepEffect);
	}

	/**
	 * Deprecated method for enabling or disabling the touch pad functionality on a DualSense controller.
	 * Use EnableTouch instead of this method for future implementations.
	 *
	 * @param ControllerId The ID of the controller to enable or disable the touch pad for.
	 * @param bEnableTouch A boolean value indicating whether to enable (true) or disable (false) the touch pad.
	 */
	UE_DEPRECATED(
		5.1, "Methods refactored and deprecated as of plugin version v1.2.1. Use EnableTouch instead of EnableTouch2.")
	UFUNCTION(BlueprintCallable, Category="DualSense Touch Pad",
		meta=(DeprecatedFunction, DeprecationMessage="Use EnableTouch"))
	static void EnableTouch2(int32 ControllerId, bool bEnableTouch)
	{
		EnableTouch(ControllerId, bEnableTouch);
	}

	/**
	 * Deprecated method for applying effects to the DualSense controller. Use the `Machine` method instead.
	 *
	 * @deprecated 5.1 Methods refactored and deprecated as of plugin version v1.2.1. Use Machine instead of EffectMachine.
	 * @param ControllerId The ID of the controller to apply the effect on.
	 * @param StartPosition The starting position for the effect, clamped between 0 and 8.
	 * @param EndPosition The ending position for the effect, clamped between 0 and 8.
	 * @param FirstFoot The first foot position of the effect, clamped between 0 and 8.
	 * @param LasFoot The last foot position of the effect, clamped between 0 and 8.
	 * @param Frequency The frequency of the effect, clamped between 0.015 and 1.0.
	 * @param Period The period of the effect, clamped between 0.015 and 1.0.
	 * @param Hand The hand (left or right) associated with the controller.
	 */
	UE_DEPRECATED(
		5.1, "Methods refactored and deprecated as of plugin version v1.2.1. Use Machine instead of EffectMachine.")
	UFUNCTION(BlueprintCallable, Category = "DualSense Effects",
		meta=(DeprecatedFunction, DeprecationMessage="Use Machine"))
	static void EffectMachine(int32 ControllerId, int32 StartPosition, int32 EndPosition, int32 FirstFoot,
	                          int32 LasFoot, float Frequency, float Period,
	                          
	                          EControllerHand Hand)
	{
		Machine(ControllerId, StartPosition, EndPosition, FirstFoot, LasFoot, Frequency, Period, Hand);
	}

	/**
	 * A deprecated method for triggering a bow effect on the DualSense controller.
	 * This method has been refactored and deprecated as of plugin version v1.2.1.
	 * Use the `Bow` method instead.
	 *
	 * @param ControllerId The ID of the controller to apply the effect on.
	 * @param StartPosition The starting position of the bow effect (range: 0-8).
	 * @param EndPosition The ending position of the bow effect (range: 0-8).
	 * @param BeginStrength The initial strength of the bow effect (range: 0-8).
	 * @param EndStrength The final strength of the bow effect (range: 0-8).
	 * @param Hand The hand (left or right) associated with the controller for the effect.
	 */
	UE_DEPRECATED(5.1, "Methods refactored and deprecated as of plugin version v1.2.1. Use Bow instead of EffectBow.")
	UFUNCTION(BlueprintCallable, Category="DualSense Effects", meta=(DeprecatedFunction, DeprecationMessage="Use Bow"))
	static void EffectBow(int32 ControllerId, int32 StartPosition, int32 EndPosition, int32 BeginStrength, int32 EndStrength,
	
	                      EControllerHand Hand)
	{
		Bow(ControllerId, StartPosition, EndPosition, BeginStrength, EndStrength, Hand);
	}

	/**
	 * Deprecated method to apply the "No Resistance" effect on a DualSense controller.
	 * This method has been deprecated as of plugin version v1.2.1. Use NoResistance instead.
	 *
	 * @param ControllerId The ID of the controller to apply the effect to.
	 * @param Hand The hand (left or right) associated with the controller.
	 */
	UE_DEPRECATED(
		5.1,
		"Methods refactored and deprecated as of plugin version v1.2.1. Use EffectNoResistance instead of EffectNoResitance.")
	UFUNCTION(BlueprintCallable, Category="DualSense Reset Effects",
		meta=(DeprecatedFunction, DeprecationMessage="Use NoResistance"))
	static void EffectNoResitance(int32 ControllerId, EControllerHand Hand)
	{
		NoResistance(ControllerId, Hand);
	}

	/**
	 * Applies section resistance feedback to the DualSense controller.
	 * This method has been deprecated and replaced by the Resistance method.
	 *
	 * @param ControllerId The ID of the controller.
	 * @param StartPosition The starting position of the resistance effect on the trigger (range: 0-8).
	 * @param EndPosition The ending position of the resistance effect on the trigger (range: 0-8).
	 * @param Strength The strength of the resistance (range: 0-8).
	 * @param ResistanceHand The hand (left or right) the resistance effect should be applied to.
	 *
	 * @deprecated Use Resistance instead.
	 */
	UE_DEPRECATED(
		5.1,
		"Methods refactored and deprecated as of plugin version v1.2.1. Use Resistance instead of EffectSectionResitance.")
	UFUNCTION(BlueprintCallable, Category="DualSense Effects",
		meta=(DeprecatedFunction, DeprecationMessage="Use Resistance"))
	static void EffectSectionResitance(int32 ControllerId, int32 StartPosition, int32 EndPosition, int32 Strength,
	                                   EControllerHand ResistanceHand)
	{
		Resistance(ControllerId, StartPosition, EndPosition, Strength, ResistanceHand);
	}

	/**
	 * Deprecated as of plugin version v1.2.1. Use ContinuousResistance instead of EffectContinuousResitance.
	 * This method applies continuous resistance to the DualSense controller.
	 *
	 * @param ControllerId The ID of the controller to which the effect should be applied.
	 * @param StartPosition The starting position of the resistance effect (range: 0-8).
	 * @param Strength The strength of the resistance effect (range: 0-8).
	 * @param ContinuousHand The hand (left or right) for which the continuous resistance effect is applied.
	 */
	UE_DEPRECATED(
		5.1,
		"Methods refactored and deprecated as of plugin version v1.2.1. Use ContinuousResistance instead of EffectContinuousResitance.")
	UFUNCTION(BlueprintCallable, Category="DualSense Effects",
		meta=(DeprecatedFunction, DeprecationMessage="Use ContinuousResistance"))
	static void EffectContinuousResitance(int32 ControllerId, int32 StartPosition, int32 Strength,
	                                      EControllerHand ContinuousHand)
	{
		ContinuousResistance(ControllerId, StartPosition, Strength, ContinuousHand);
	}

	/**
	 * A deprecated function that applies a weapon effect on a DualSense controller. This method has been refactored
	 * and deprecated as of plugin version v1.2.1. Use the `Weapon` method instead.
	 *
	 * @param ControllerId The ID of the controller on which to apply the effect.
	 * @param StartPosition The starting position of the effect on the controller's trigger, in the range [0, 8].
	 * @param EndPosition The ending position of the effect on the controller's trigger, in the range [0, 8].
	 * @param Strength The intensity of the effect, in the range [0, 8].
	 * @param Hand The hand (left or right) to which the effect should be applied on the controller.
	 */
	UE_DEPRECATED(
		5.1, "Methods refactored and deprecated as of plugin version v1.2.1. Use Weapon instead of EffectWeapon.")
	UFUNCTION(BlueprintCallable, Category = "DualSense Effects",
		meta=(DeprecatedFunction, DeprecationMessage="Use Weapon"))
	static void EffectWeapon(int32 ControllerId, int32 StartPosition, int32 EndPosition, int32 Strength,
	                         EControllerHand Hand)
	{
		Weapon(ControllerId, StartPosition, EndPosition, Strength, Hand);
	}

	/**
	 * Deprecated method for triggering the "Galloping" effect on a DualSense controller.
	 * As of version 1.2.1 of the plugin, this method has been refactored and is deprecated.
	 * Use the `Galloping` method instead.
	 *
	 * @param ControllerId The ID of the DualSense controller.
	 * @param StartPosition The initial position of the effect (0-8).
	 * @param EndPosition The final position of the effect (0-8).
	 * @param BeginStrength The strength of the effect at the start position (0-8).
	 * @param EndStrength The strength of the effect at the end position (0-8).
	 * @param Frequency The frequency of the effect (0.01-1.0).
	 * @param Hand The controller hand (left or right).
	 */
	UE_DEPRECATED(
		5.1, "Methods refactored and deprecated as of plugin version v1.2.1. Use Galloping instead of EffectGalloping.")
	UFUNCTION(BlueprintCallable, Category = "DualSense Effects",
		meta=(DeprecatedFunction, DeprecationMessage="Use Galloping"))
	static void EffectGalloping(int32 ControllerId, int32 StartPosition, int32 EndPosition, int32 BeginStrength,
	                            int32 EndStrength, float Frequency, EControllerHand Hand)
	{
		Galloping(ControllerId, StartPosition, EndPosition, BeginStrength, EndStrength, Frequency, Hand);
	}

	

	/**
	 * Sets the vibration for a DualSense controller based on audio envelopes and other parameters.
	 *
	 * @param ControllerId The ID of the controller to apply the vibration to.
	 * @param AverageEnvelopeValue The average audio envelope value, used to calculate the left vibration intensity.
	 * @param MaxEnvelopeValue The maximum audio envelope value, used to calculate the right vibration intensity.
	 * @param NumWaveInstances The number of wave instances contributing to the audio signal, affecting the overall vibration strength.
	 * @param EnvelopeToVibrationMultiplier Multiplier to scale the average envelope value to vibration intensity.
	 * @param PeakToVibrationMultiplier Multiplier to scale the maximum envelope value to vibration intensity.
	 * @param Threshold The minimum vibration level threshold for activation.
	 * @param ExponentCurve The exponent curve used to shape the vibration scaling.
	 * @param BaseMultiplier A base multiplier applied to vibration for additional scaling.
	 */
	UE_DEPRECATED(
		5.1, "Methods refactored and deprecated as of plugin version v1.2.18. Use RegisterSubmixForDevice(int32 ControllerId, USoundSubmix* Submix).")
	UFUNCTION(BlueprintCallable, Category = "DualSense Audio Vibration", meta=(DeprecatedFunction, DeprecationMessage="Use RegisterSubmixForDevice(int32 ControllerId, USoundSubmix* Submix)"))
	void SetVibrationFromAudio(
		const int32 ControllerId,
		const float AverageEnvelopeValue,
		const float MaxEnvelopeValue,
		const int32 NumWaveInstances,
		float EnvelopeToVibrationMultiplier,
		float PeakToVibrationMultiplier,
		float Threshold,
		float ExponentCurve,
		float BaseMultiplier
	)
	{
	}

	/**
	 * Retrieves the strength of the right trigger feedback for the specified controller.
	 *
	 * @param ControllerId The unique identifier for the controller whose right trigger feedback strength is to be retrieved.
	 * @return The strength value of the right trigger feedback. Returns 0 if the controller instance is not found.
	 */
	UFUNCTION(BlueprintCallable, Category = "DualSense Status")
	static int32 GetTriggerRightStrengthFeedback(int32 ControllerId)
	{
		return 0;
	}

	/**
	 * Retrieves the strength of the left trigger feedback for the specified DualSense controller.
	 *
	 * @param ControllerId The ID of the controller to retrieve the feedback strength for.
	 * @return The strength of the left trigger feedback as an integer. Returns 0 if the controller instance is not found.
	 */
	UFUNCTION(BlueprintCallable, Category = "DualSense Status")
	int32 GetTriggerLeftStrengthFeedback(int32 ControllerId)
	{
		return 0;
	}

	
	
};
