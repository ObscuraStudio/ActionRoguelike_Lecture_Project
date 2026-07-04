// Copyright (c) 2025 Rafael Valoto/Publisher. All rights reserved.
// Created for: WindowsDualsense_ds5w - Plugin to support DualSense controller on Windows.
// Planned Release Year: 2025

#include "../../Public/Core/HapticsRegistry.h"
#include "Misc/App.h"
#include "Runtime/Launch/Resources/Version.h"
#include "AudioDevice.h"
#include "Async/Async.h"

TSharedPtr<FHapticsRegistry> FHapticsRegistry::Instance;

FHapticsRegistry::~FHapticsRegistry()
{
	RemoveAllListeners();
	if (Instance->GameThreadTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(Instance->GameThreadTickerHandle);
	}
}

bool FHapticsRegistry::HasListenerForDevice(const FInputDeviceId& DeviceId) const
{
	return ControllerListeners.Contains(DeviceId);
}

TSharedPtr<FHapticsRegistry> FHapticsRegistry::Get()
{
	if (!Instance)
	{
		check(IsInGameThread());
		Instance = MakeShared<FHapticsRegistry>();

		Instance->GameThreadTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		   FTickerDelegate::CreateSP(Instance.Get(), &FHapticsRegistry::Tick)
	    );
	}
	return Instance;
}

void FHapticsRegistry::CreateListenerForDevice(const FInputDeviceId& DeviceId, USoundSubmix* Submix)
{
	if (!Submix) return;

	if (ControllerListeners.Contains(DeviceId))
	{
		UE_LOG(LogTemp, Log, TEXT("Haptics listener already registered for device %d"), DeviceId.GetId());
		RemoveListenerForDevice(DeviceId);
	};
	
	const TSharedPtr<FAudioHapticsListener> Listener = MakeShared<FAudioHapticsListener>(DeviceId, Submix);
	if (FAudioDeviceHandle AudioDevice = GEngine->GetActiveAudioDevice())
	{
#if ENGINE_MINOR_VERSION > 3 && ENGINE_MAJOR_VERSION == 5
		AudioDevice->RegisterSubmixBufferListener(Listener.ToSharedRef(), *Submix);
#else
		AudioDevice->RegisterSubmixBufferListener(Listener.Get(), Submix);
#endif
		UE_LOG(LogTemp, Log, TEXT("Registering listener for device %d num %d"), DeviceId.GetId(), ControllerListeners.Num());

		ControllerListeners.Add(DeviceId, Listener);
	}
}


void FHapticsRegistry::RemoveAllListeners()
{
	if (FAudioDeviceHandle AudioDevice = GEngine->GetActiveAudioDevice())
	{
		for (auto& Pair : ControllerListeners)
		{
#if ENGINE_MINOR_VERSION > 3 && ENGINE_MAJOR_VERSION == 5
			AudioDevice->UnregisterSubmixBufferListener(Pair.Value.ToSharedRef(), *Pair.Value.ToSharedRef()->GetSubmix());
#else
			AudioDevice->UnregisterSubmixBufferListener(Pair.Value.Get());
#endif
		}
	}
	ControllerListeners.Empty();
}

bool FHapticsRegistry::Tick(float DeltaTime)
{
	for (auto& Pair : ControllerListeners)
	{
		if (Pair.Value.IsValid())
		{
			TSharedPtr<FAudioHapticsListener> Context = Pair.Value;
			AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [NewContext = MoveTemp(Context)]()
			{
				NewContext->ConsumeHapticsQueue();
			});
		}
	}
	return true;
}

void FHapticsRegistry::RemoveListenerForDevice(const FInputDeviceId& DeviceId)
{
	if (const TSharedPtr<FAudioHapticsListener>* ExistingListener = ControllerListeners.Find(DeviceId))
	{
		if (FAudioDeviceHandle AudioDevice = GEngine->GetActiveAudioDevice())
		{
#if ENGINE_MINOR_VERSION > 3 && ENGINE_MAJOR_VERSION == 5
			AudioDevice->UnregisterSubmixBufferListener(ExistingListener->ToSharedRef(), *ExistingListener->ToSharedRef()->GetSubmix());
#else
			AudioDevice->UnregisterSubmixBufferListener(ExistingListener->Get());
#endif
			UE_LOG(LogTemp, Log, TEXT("Unregistered haptics listener for device %d"), DeviceId.GetId());
		}
		ControllerListeners.Remove(DeviceId);
	}
}
