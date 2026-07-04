// Copyright (c) 2025 Rafael Valoto/Publisher. All rights reserved.
// Created for: WindowsDualsense_ds5w - Plugin to support DualSense controller on Windows.
// Planned Release Year: 2025

#include "DualSenseProxy.h"

#include "AudioDevice.h"
#include "Core/DeviceRegistry.h"
#include "Core/DualSense/DualSenseLibrary.h"
#include "Core/Interfaces/SonyGamepadInterface.h"
#include "Core/Interfaces/SonyGamepadTriggerInterface.h"
#include "Helpers/ValidateHelpers.h"


void UDualSenseProxy::DeviceSettings(int32 ControllerId, FDualSenseFeatureReport Settings)
{
	const FInputDeviceId DeviceId = GetGamepadInterface(ControllerId);
	if (!DeviceId.IsValid())
	{
		return;
	}

	ISonyGamepadInterface* Gamepad = FDeviceRegistry::Get()->GetLibraryInstance(DeviceId);
	if (!Gamepad)
	{
		return;
	}

	UDualSenseLibrary* DualSenseInstance = Cast<UDualSenseLibrary>(Gamepad);
	if (!DualSenseInstance)
	{
		return;
	}
	DualSenseInstance->Settings(Settings);
}

void UDualSenseProxy::RegisterSubmixForDevice(int32 ControllerId, USoundSubmix* Submix)
{
	const FInputDeviceId DeviceId = GetGamepadInterface(ControllerId);
	if (!DeviceId.IsValid())
	{
		return;
	}

	FHapticsRegistry::Get()->CreateListenerForDevice(DeviceId, Submix);
}

void UDualSenseProxy::UnregisterSubmixForDevice(int32 ControllerId)
{
	const FInputDeviceId DeviceId = GetGamepadInterface(ControllerId);
	if (!DeviceId.IsValid())
	{
		return;
	}
	FHapticsRegistry::Get()->RemoveListenerForDevice(DeviceId);
}

void UDualSenseProxy::LedPlayerEffects(int32 ControllerId, ELedPlayerEnum Value, ELedBrightnessEnum Brightness)
{
	const FInputDeviceId DeviceId = GetGamepadInterface(ControllerId);
	if (!DeviceId.IsValid())
	{
		return;
	}

	ISonyGamepadInterface* Gamepad = FDeviceRegistry::Get()->GetLibraryInstance(DeviceId);
	if (!Gamepad)
	{
		return;
	}

	Gamepad->SetPlayerLed(Value, Brightness);
}



void UDualSenseProxy::SetFeedback(int32 ControllerId, int32 BeginStrength,
                                  int32 MiddleStrength, int32 EndStrength, EControllerHand Hand)
{
	if (!FValidateHelpers::ValidateMaxPosition(BeginStrength)) BeginStrength = 8;
	if (!FValidateHelpers::ValidateMaxPosition(MiddleStrength)) MiddleStrength = 8;
	if (!FValidateHelpers::ValidateMaxPosition(EndStrength)) EndStrength = 8;

	const FInputDeviceId DeviceId = GetGamepadInterface(ControllerId);
	if (!DeviceId.IsValid())
	{
		return;
	}

	ISonyGamepadTriggerInterface* Gamepad = Cast<ISonyGamepadTriggerInterface>(FDeviceRegistry::Get()->GetLibraryInstance(DeviceId));
	if (!Gamepad)
	{
		return;
	}

	return Gamepad->SetResistance(BeginStrength, MiddleStrength, EndStrength, Hand);
}

void UDualSenseProxy::Resistance(int32 ControllerId, int32 StartPosition, int32 EndPosition, int32 Strength, EControllerHand Hand)
{
	if (!FValidateHelpers::ValidateMaxPosition(StartPosition)) StartPosition = 0;
	if (!FValidateHelpers::ValidateMaxPosition(EndPosition)) EndPosition = 0;
	if (!FValidateHelpers::ValidateMaxPosition(Strength)) Strength = 0;

	const FInputDeviceId DeviceId = GetGamepadInterface(ControllerId);
	if (!DeviceId.IsValid())
	{
		return;
	}

	ISonyGamepadTriggerInterface* Gamepad = Cast<ISonyGamepadTriggerInterface>(FDeviceRegistry::Get()->GetLibraryInstance(DeviceId));
	if (!Gamepad)
	{
		return;
	}

	Gamepad->SetResistance(StartPosition, EndPosition, Strength, Hand);
}

void UDualSenseProxy::AutomaticGun(int32 ControllerId, int32 BeginStrength, int32 MiddleStrength, int32 EndStrength, EControllerHand Hand, bool KeepEffect, float Frequency)
{
	if (!FValidateHelpers::ValidateMaxPosition(BeginStrength)) BeginStrength = 6;
	if (!FValidateHelpers::ValidateMaxPosition(MiddleStrength)) MiddleStrength = 8;
	if (!FValidateHelpers::ValidateMaxPosition(EndStrength)) EndStrength = 8;

	const FInputDeviceId DeviceId = GetGamepadInterface(ControllerId);
	if (!DeviceId.IsValid())
	{
		return;
	}

	ISonyGamepadTriggerInterface* Gamepad = Cast<ISonyGamepadTriggerInterface>(FDeviceRegistry::Get()->GetLibraryInstance(DeviceId));
	if (!Gamepad)
	{
		return;
	}

	Gamepad->SetAutomaticGun(BeginStrength, MiddleStrength, EndStrength, Hand, KeepEffect, Frequency);
}

void UDualSenseProxy::GameCube(int32 ControllerId, EControllerHand Hand)
{
	const FInputDeviceId DeviceId = GetGamepadInterface(ControllerId);
	if (!DeviceId.IsValid())
	{
		return;
	}

	ISonyGamepadTriggerInterface* Gamepad = Cast<ISonyGamepadTriggerInterface>(FDeviceRegistry::Get()->GetLibraryInstance(DeviceId));
	if (!Gamepad)
	{
		return;
	}
	Gamepad->SetGameCube(Hand);
}

void UDualSenseProxy::CustomTrigger(int32 ControllerId, EControllerHand Hand, const TArray<FString>& HexBytes)
{
	const FInputDeviceId DeviceId = GetGamepadInterface(ControllerId);
	if (!DeviceId.IsValid())
	{
		return;
	}

	ISonyGamepadTriggerInterface* Gamepad = Cast<ISonyGamepadTriggerInterface>(FDeviceRegistry::Get()->GetLibraryInstance(DeviceId));
	if (!Gamepad)
	{
		return;
	}

	if (HexBytes.Num() > 10)
	{
		return;
	}
	Gamepad->CustomTrigger(Hand, HexBytes);
}

void UDualSenseProxy::ContinuousResistance(int32 ControllerId, int32 StartPosition, int32 Strength, EControllerHand Hand)
{
	if (!FValidateHelpers::ValidateMaxPosition(StartPosition)) StartPosition = 0;
	if (!FValidateHelpers::ValidateMaxPosition(Strength)) Strength = 8;

	const FInputDeviceId DeviceId = GetGamepadInterface(ControllerId);
	if (!DeviceId.IsValid())
	{
		return;
	}

	ISonyGamepadTriggerInterface* Gamepad = Cast<ISonyGamepadTriggerInterface>(FDeviceRegistry::Get()->GetLibraryInstance(DeviceId));
	if (!Gamepad)
	{
		return;
	}

	Gamepad->SetContinuousResistance(StartPosition, Strength, Hand);
}

void UDualSenseProxy::Galloping(
	int32 ControllerId, int32 StartPosition, int32 EndPosition, int32 FirstFoot,
                                int32 SecondFoot, float Frequency, EControllerHand Hand)
{
	if (!FValidateHelpers::ValidateMaxPosition(StartPosition, 8, 1)) StartPosition = 1;
	if (!FValidateHelpers::ValidateMaxPosition(EndPosition, 9, StartPosition+1)) EndPosition = 9;
	if (!FValidateHelpers::ValidateMaxPosition(FirstFoot, 8)) FirstFoot = 1;
	if (!FValidateHelpers::ValidateMaxPosition(SecondFoot, 9, FirstFoot+1)) SecondFoot = 9;

	const FInputDeviceId DeviceId = GetGamepadInterface(ControllerId);
	if (!DeviceId.IsValid())
	{
		return;
	}

	ISonyGamepadTriggerInterface* Gamepad = Cast<ISonyGamepadTriggerInterface>(FDeviceRegistry::Get()->GetLibraryInstance(DeviceId));
	if (!Gamepad)
	{
		return;
	}

	Gamepad->SetGalloping(StartPosition, EndPosition, FirstFoot, SecondFoot, Frequency, Hand);
}

void UDualSenseProxy::Machine(int32 ControllerId, int32 StartPosition, int32 EndPosition, int32 FirstFoot,
                              int32 LasFoot, float Frequency, float Period, EControllerHand Hand)
{
	if (!FValidateHelpers::ValidateMaxPosition(StartPosition)) StartPosition = 2;
	if (StartPosition < 2)
	{
		StartPosition = 2;
	}

	if (!FValidateHelpers::ValidateMaxPosition(EndPosition)) EndPosition = 8;
	if (!FValidateHelpers::ValidateMaxPosition(FirstFoot)) FirstFoot = 1;
	if (!FValidateHelpers::ValidateMaxPosition(LasFoot)) LasFoot = 8;

	const FInputDeviceId DeviceId = GetGamepadInterface(ControllerId);
	if (!DeviceId.IsValid())
	{
		return;
	}

	ISonyGamepadTriggerInterface* Gamepad = Cast<ISonyGamepadTriggerInterface>(FDeviceRegistry::Get()->GetLibraryInstance(DeviceId));
	if (!Gamepad)
	{
		return;
	}

	Gamepad->SetMachine(StartPosition, EndPosition, FirstFoot, LasFoot, Frequency, Period, Hand);
}

void UDualSenseProxy::Weapon(int32 ControllerId, int32 StartPosition, int32 EndPosition, int32 Strength,
	EControllerHand Hand)
{
	if (!FValidateHelpers::ValidateMaxPosition(StartPosition)) StartPosition = 0;
	if (!FValidateHelpers::ValidateMaxPosition(EndPosition)) EndPosition = 8;
	if (!FValidateHelpers::ValidateMaxPosition(Strength)) Strength = 8;

	const FInputDeviceId DeviceId = GetGamepadInterface(ControllerId);
	if (!DeviceId.IsValid())
	{
		return;
	}

	ISonyGamepadTriggerInterface* Gamepad = Cast<ISonyGamepadTriggerInterface>(FDeviceRegistry::Get()->GetLibraryInstance(DeviceId));
	if (!Gamepad)
	{
		return;
	}

	Gamepad->SetWeapon(StartPosition, EndPosition, Strength, Hand);
}

void UDualSenseProxy::Bow(int32 ControllerId, int32 StartPosition, int32 EndPosition, int32 BeginStrength, int32 EndStrength,
                          EControllerHand Hand)
{
	if (!FValidateHelpers::ValidateMaxPosition(StartPosition)) StartPosition = 2;
	if (!FValidateHelpers::ValidateMaxPosition(BeginStrength)) BeginStrength = 8;
	if (!FValidateHelpers::ValidateMaxPosition(EndPosition)) EndPosition = 8;
	if (!FValidateHelpers::ValidateMaxPosition(EndStrength)) EndStrength = 8;

	const FInputDeviceId DeviceId = GetGamepadInterface(ControllerId);
	if (!DeviceId.IsValid())
	{
		return;
	}

	ISonyGamepadTriggerInterface* Gamepad = Cast<ISonyGamepadTriggerInterface>(FDeviceRegistry::Get()->GetLibraryInstance(DeviceId));
	if (!Gamepad)
	{
		return;
	}

	Gamepad->SetBow(StartPosition, EndPosition, BeginStrength, EndStrength, Hand);
}

void UDualSenseProxy::NoResistance(int32 ControllerId, EControllerHand Hand)
{
	const FInputDeviceId DeviceId = GetGamepadInterface(ControllerId);
	if (!DeviceId.IsValid())
	{
		return;
	}

	ISonyGamepadTriggerInterface* Gamepad = Cast<ISonyGamepadTriggerInterface>(FDeviceRegistry::Get()->GetLibraryInstance(DeviceId));
	if (!Gamepad)
	{
		return;
	}

	Gamepad->StopTrigger(Hand);
}

void UDualSenseProxy::StopTriggerEffect(const int32 ControllerId, EControllerHand HandStop)
{
	const FInputDeviceId DeviceId = GetGamepadInterface(ControllerId);
	if (!DeviceId.IsValid())
	{
		return;
	}

	ISonyGamepadTriggerInterface* Gamepad = Cast<ISonyGamepadTriggerInterface>(FDeviceRegistry::Get()->GetLibraryInstance(DeviceId));
	if (!Gamepad)
	{
		return;
	}

	Gamepad->StopTrigger(HandStop);
}

void UDualSenseProxy::StopAllTriggersEffects(const int32 ControllerId)
{
	const FInputDeviceId DeviceId = GetGamepadInterface(ControllerId);
	if (!DeviceId.IsValid())
	{
		return;
	}

	ISonyGamepadTriggerInterface* Gamepad = Cast<ISonyGamepadTriggerInterface>(FDeviceRegistry::Get()->GetLibraryInstance(DeviceId));
	if (!Gamepad)
	{
		return;
	}

	Gamepad->StopTrigger(EControllerHand::AnyHand);
}

void UDualSenseProxy::ResetEffects(const int32 ControllerId)
{
	const FInputDeviceId DeviceId = GetGamepadInterface(ControllerId);
	if (!DeviceId.IsValid())
	{
		return;
	}

	ISonyGamepadInterface* Gamepad = FDeviceRegistry::Get()->GetLibraryInstance(DeviceId);
	if (!Gamepad)
	{
		return;
	}

	Gamepad->StopAll();
}
