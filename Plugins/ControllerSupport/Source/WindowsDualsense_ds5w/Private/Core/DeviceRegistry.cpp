// Copyright (c) 2025 Rafael Valoto/Publisher. All rights reserved.
// Created for: WindowsDualsense_ds5w - Plugin to support DualSense controller on Windows.
// Planned Release Year: 2025

#include "Core/DeviceRegistry.h"
#include "Async/Async.h"
#include "Async/TaskGraphInterfaces.h"
#include "HAL/PlatformProcess.h"
#include "Core/DualSense/DualSenseLibrary.h"
#include "Core/DualShock/DualShockLibrary.h"
#include "Core/Interfaces/PlatformHardwareInfoInterface.h"
#include "GameFramework/InputSettings.h"
#include "Core/Structs/DeviceContext.h"
#include "Core/Structs/OutputContext.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Core/Interfaces/SonyGamepadInterface.h"

// #if PLATFORM_WINDOWS
// #include "Windows/WindowsApplication.h"
// #endif

TSharedPtr<FDeviceRegistry> FDeviceRegistry::Instance;
TMap<FString, FInputDeviceId> FDeviceRegistry::KnownDevicePaths;
TMap<FString, FInputDeviceId> FDeviceRegistry::HistoryDevices;
TMap<FInputDeviceId, ISonyGamepadInterface*> FDeviceRegistry::LibraryInstances;

bool PrimaryTick = true;
void FDeviceRegistry::DetectedChangeConnections(float DeltaTime)
{
	if (!PrimaryTick)
	{
		AccumulatorDelta += DeltaTime;
		if (AccumulatorDelta < 2.0f || bIsDeviceDetectionInProgress)
		{
			return;
		}

		AccumulatorDelta = 0.0f;
	}

	PrimaryTick = false;
	bIsDeviceDetectionInProgress = true;

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [WeakManager = AsWeak()]()
	{
			TArray<FDeviceContext> DetectedDevices;
			DetectedDevices.Reset();

			IPlatformHardwareInfoInterface::Get().Detect(DetectedDevices);
			AsyncTask(ENamedThreads::GameThread, [WeakManager, DetectedDevices = MoveTemp(DetectedDevices)]() mutable
			{
				const TSharedPtr<FDeviceRegistry> Manager = WeakManager.Pin();
				if (!Manager)
				{
					return;
				}
			
				TSet<FString> CurrentlyConnectedPaths;
				for (const FDeviceContext& Context : DetectedDevices)
				{
					CurrentlyConnectedPaths.Add(Context.Path);
				}

				TArray<FString> DisconnectedPaths;
				for (const TPair<FString, FInputDeviceId>& KnownDevice : Manager->KnownDevicePaths)
				{
					const FString& Path = KnownDevice.Key;
					const FInputDeviceId& DeviceId = KnownDevice.Value;
					if (!CurrentlyConnectedPaths.Contains(Path))
					{
						if (Manager->LibraryInstances.Contains(DeviceId))
						{
							Manager->RemoveLibraryInstance(DeviceId);
							DisconnectedPaths.Add(Path);
						}
					}
				}

				for (const FString& Path : DisconnectedPaths)
				{
					if (Manager->KnownDevicePaths.Contains(Path))
					{
						Manager->KnownDevicePaths.Remove(Path);
					}
				}

				for (FDeviceContext& Context : DetectedDevices)
				{
					if (!Manager->KnownDevicePaths.Contains(Context.Path))
					{
						Context.Output = FOutputContext();
						if (!IPlatformHardwareInfoInterface::Get().CreateHandle(&Context))
						{
							UE_LOG(LogTemp, Log, TEXT("DualSense: DeviceManager Failed to create handle for device %s."), *Context.Path);
							continue;
						}
						
						if (Context.Handle == INVALID_PLATFORM_HANDLE)
						{
							continue;
						}
						Manager->CreateLibraryInstance(Context);
					}
				}
				
				Manager->bIsDeviceDetectionInProgress = false;
			});
	});
}

TSharedPtr<FDeviceRegistry> FDeviceRegistry::Get()
{
	if (!Instance)
	{
		check(IsInGameThread());
		Instance = MakeShared<FDeviceRegistry>();
	}
	return Instance;
}

FDeviceRegistry::~FDeviceRegistry()
{
	TArray<FInputDeviceId> WatcherKeys;
	LibraryInstances.GetKeys(WatcherKeys);

	for (const FInputDeviceId& ControllerId : WatcherKeys)
	{
		RemoveLibraryInstance(ControllerId);
	}
}

ISonyGamepadInterface* FDeviceRegistry::GetLibraryInstance(const FInputDeviceId& DeviceId)
{
	if (!LibraryInstances.Contains(DeviceId))
	{
		return nullptr;
	}

	if (!LibraryInstances[DeviceId]->IsConnected())
	{
		return nullptr;
	}
	return LibraryInstances[DeviceId];
}

void FDeviceRegistry::RemoveLibraryInstance(const FInputDeviceId& GamepadId)
{
	// We should never call into IPlatformInputDeviceMapper from non-game thread because it is not thread-safe
	check(IsInGameThread());

	if (!LibraryInstances.Contains(GamepadId))
	{
		return;
	}

	IPlatformInputDeviceMapper::Get().Internal_SetInputDeviceConnectionState(GamepadId, EInputDeviceConnectionState::Disconnected);

	LibraryInstances[GamepadId]->ShutdownLibrary();
	LibraryInstances.Remove(GamepadId);
}

void FDeviceRegistry::CreateLibraryInstance(FDeviceContext& Context)
{
	ISonyGamepadInterface* SonyGamepad = nullptr;
	if (Context.DeviceType == EDeviceType::DualSense || Context.DeviceType == EDeviceType::DualSenseEdge)
	{
		SonyGamepad = Cast<ISonyGamepadInterface>(NewObject<UDualSenseLibrary>(UDualSenseLibrary::StaticClass()));
	}

	if (Context.DeviceType == EDeviceType::DualShock4)
	{
		SonyGamepad = Cast<ISonyGamepadInterface>(NewObject<UDualShockLibrary>(UDualShockLibrary::StaticClass()));
	}

	if (!SonyGamepad)
	{
		return;
	}

	TArray<FInputDeviceId> Devices;
	Devices.Reset();
	
	// We should never call into IPlatformInputDeviceMapper from non-game thread because it is not thread-safe
	check(IsInGameThread());

	IPlatformInputDeviceMapper::Get().GetAllInputDevicesForUser(
		IPlatformInputDeviceMapper::Get().GetPrimaryPlatformUser(), Devices);

	bool AllocateDeviceToDefaultUser = false;
	if (Devices.Num() <= 1)
	{
		AllocateDeviceToDefaultUser = true;
	}

	const FName UniqueNamespace = TEXT("DeviceManager.WindowsDualsense");
	const FHardwareDeviceIdentifier HardwareId(UniqueNamespace, *Context.Path);
	if (HistoryDevices.Contains(Context.Path))
	{
		Context.UniqueInputDeviceId = HistoryDevices[Context.Path];
	}
	else
	{
		Context.UniqueInputDeviceId = IPlatformInputDeviceMapper::Get().AllocateNewInputDeviceId();
		HistoryDevices.Add(Context.Path, Context.UniqueInputDeviceId);
	}

	SonyGamepad->_getUObject()->AddToRoot();
	SonyGamepad->InitializeLibrary(Context);
	
	KnownDevicePaths.Add(Context.Path, Context.UniqueInputDeviceId);
	LibraryInstances.Add(Context.UniqueInputDeviceId, SonyGamepad);

	FInputDeviceId GamepadId = Context.UniqueInputDeviceId;
	if (
		IPlatformInputDeviceMapper::Get().GetInputDeviceConnectionState(GamepadId) !=
		EInputDeviceConnectionState::Connected)
	{
		FPlatformUserId UserId = IPlatformInputDeviceMapper::Get().GetUserForInputDevice(GamepadId);
		if (!UserId.IsValid())
		{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 6
			UserId = AllocateDeviceToDefaultUser
				? IPlatformInputDeviceMapper::Get().GetPrimaryPlatformUser()
				: IPlatformInputDeviceMapper::Get().AllocateNewUserId();

#else
			UserId = IPlatformInputDeviceMapper::Get().GetPlatformUserForNewlyConnectedDevice();
#endif
		}

		IPlatformInputDeviceMapper::Get().Internal_MapInputDeviceToUser(Context.UniqueInputDeviceId,
		                                                                UserId,
		                                                                EInputDeviceConnectionState::Connected);
	}
}

int32 FDeviceRegistry::GetAllocatedDevices()
{
	return LibraryInstances.Num();
}

TMap<FInputDeviceId, ISonyGamepadInterface*> FDeviceRegistry::GetAllocatedDevicesMap()
{
	return LibraryInstances;
}
