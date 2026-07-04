// Copyright (c) 2025 Rafael Valoto/Publisher. All rights reserved.
// Created for: WindowsDualsense_ds5w - Plugin to support DualSense controller on Windows.
// Planned Release Year: 2025

#include "DeviceManager.h"
#include "Async/Async.h"
#include "Async/TaskGraphInterfaces.h"
#include "Core/DeviceRegistry.h"
#include "Core/Interfaces/SonyGamepadTriggerInterface.h"
#include "GameFramework/InputDeviceSubsystem.h"
#include "Misc/CoreDelegates.h"

DeviceManager::DeviceManager(
	const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler
	): MessageHandler(InMessageHandler)
{
	FCoreDelegates::OnUserLoginChangedEvent.AddRaw(this, &DeviceManager::OnUserLoginChangedEvent);
}

DeviceManager::~DeviceManager()
{
	FCoreDelegates::OnUserLoginChangedEvent.RemoveAll(this);
}

void DeviceManager::SendControllerEvents()
{
	
}

void DeviceManager::Tick(float DeltaTime)
{
	FDeviceRegistry::Get()->DetectedChangeConnections(DeltaTime);
	
	PollAccumulator += DeltaTime;
	if (PollAccumulator < PollInterval)
	{
		return;
	}
	
	PollAccumulator = 0.0f;

	TArray<FInputDeviceId> OutInputDevices;
	OutInputDevices.Reset();
	IPlatformInputDeviceMapper::Get().GetAllConnectedInputDevices(OutInputDevices);
	for (const FInputDeviceId& Device : OutInputDevices)
	{
		if (ISonyGamepadInterface* Gamepad = FDeviceRegistry::Get()->GetLibraryInstance(Device); Gamepad)
		{
			const FPlatformUserId UserId = IPlatformInputDeviceMapper::Get().GetUserForInputDevice(Device);
			if (const int32 ControllerId = FPlatformMisc::GetUserIndexForPlatformUser(UserId); ControllerId == -1)
			{
				continue;
			}

			FString ContextDrive = TEXT("DualSense");
			if (Gamepad->GetDeviceType() == EDeviceType::DualShock4)
			{
				ContextDrive = TEXT("DualShock4");
			}
			if (Gamepad->GetDeviceType() == EDeviceType::DualSenseEdge)
			{
				ContextDrive = TEXT("DualSenseEdge");
			}

			FInputDeviceScope InputScope(this, TEXT("DeviceManager.WindowsDualsense"), Device.GetId(), ContextDrive);
			Gamepad->UpdateInput(MessageHandler, UserId, Device, DeltaTime);
			if (Gamepad->IsSendControllerEvents())
			{
				UInputDeviceSubsystem::Get()->OnInputHardwareDeviceChanged.Broadcast(UserId, Device);
				Gamepad->SetControllerEvents(false);
			}
		}
	}
	
}

void DeviceManager::SetDeviceProperty(int32 ControllerId, const FInputDeviceProperty* Property)
{
	if (!Property) return;

	if (Property->Name == "InputDeviceLightColor")
	{
		const FInputDeviceLightColorProperty* ColorProperty = static_cast<const FInputDeviceLightColorProperty*>(Property);
		SetLightColor(ControllerId, ColorProperty->Color);
	}

	if (Property->Name == FName("InputDeviceTriggerResistance"))
	{
		const FInputDeviceId DeviceId = GetGamepadInterface(ControllerId);
		if (!DeviceId.IsValid())
		{
			return;
		}
		
		ISonyGamepadTriggerInterface* GamepadTrigger = Cast<ISonyGamepadTriggerInterface>(FDeviceRegistry::Get()->GetLibraryInstance(DeviceId));
		if (!IsValid(GamepadTrigger->_getUObject()))
		{
			return;
		}
		GamepadTrigger->SetTriggers(Property);
	}
}

void DeviceManager::SetHapticFeedbackValues(const int32 ControllerId, const int32 Hand, const FHapticFeedbackValues& Values)
{
	UE_LOG(LogTemp, Warning, TEXT("SetHapticFeedbackValues"));
	const FInputDeviceId DeviceId = GetGamepadInterface(ControllerId);
	if (!DeviceId.IsValid())
	{
		return;
	}

	ISonyGamepadTriggerInterface* GamepadTrigger = Cast<ISonyGamepadTriggerInterface>(FDeviceRegistry::Get()->GetLibraryInstance(DeviceId));
	if (!GamepadTrigger)
	{
		return;
	}
	
	GamepadTrigger->SetHapticFeedback(Hand, &Values);
}

void DeviceManager::SetChannelValues(int32 ControllerId, const FForceFeedbackValues& Values)
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
	
	Gamepad->SetVibration(Values);
}

void DeviceManager::SetLightColor(const int32 ControllerId, const FColor Color)
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

	Gamepad->SetLightbar(Color);
}

void DeviceManager::ResetLightColor(const int32 ControllerId)
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
	
	Gamepad->SetLightbar(FColor::Blue);
}

void DeviceManager::OnUserLoginChangedEvent(bool bLoggedIn, int32 UserId, int32 UserIndex) const
{
	const FPlatformUserId PlatformUserId = FPlatformUserId::CreateFromInternalId(UserId);
	if (!bLoggedIn)
	{
		TArray<FInputDeviceId> OutInputDevices;
		OutInputDevices.Reset();

		IPlatformInputDeviceMapper::Get().GetAllInputDevicesForUser(PlatformUserId, OutInputDevices);
		for (const FInputDeviceId& DeviceId : OutInputDevices)
		{
			IPlatformInputDeviceMapper::Get().Internal_MapInputDeviceToUser(DeviceId, PlatformUserId, EInputDeviceConnectionState::Disconnected);
		}
	}
}

FInputDeviceId DeviceManager::GetGamepadInterface(int32 ControllerId)
{
	TArray<FInputDeviceId> Devices;
	
	IPlatformInputDeviceMapper::Get().GetAllInputDevicesForUser(FPlatformUserId::CreateFromInternalId(ControllerId), Devices);
	
	for (const FInputDeviceId& DeviceId : Devices)
	{
		if (FDeviceRegistry::Get()->GetLibraryInstance(DeviceId))
		{
			return DeviceId;
		}
	}

	return FInputDeviceId::CreateFromInternalId(INDEX_NONE);
} 
