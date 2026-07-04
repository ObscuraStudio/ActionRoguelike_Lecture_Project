// Copyright (c) 2025 Rafael Valoto/Publisher. All rights reserved.
// Created for: WindowsDualsense_ds5w - Plugin to support DualSense controller on Windows.
// Planned Release Year: 2025

#include "../../../../Public/Core/Platforms/Commons/CommonsDeviceInfo.h"

#if PLATFORM_WINDOWS
#else
#include "SDL_hidapi.h"

static const uint16 SONY_VENDOR_ID = 0x054C;
static const uint16 DUALSHOCK4_PID_V1 = 0x05C4;
static const uint16 DUALSHOCK4_PID_V2 = 0x09CC;
static const uint16 DUALSENSE_PID     = 0x0CE6;
static const uint16 DUALSENSE_EDGE_PID= 0x0DF2;

void FCommonsDeviceInfo::Read(FDeviceContext* Context)
{
	if (!Context || !Context->Handle)
	{
		return;
	}

	int BytesRead = 0;
	if (Context->ConnectionType == Bluetooth && Context->DeviceType == EDeviceType::DualShock4)
	{
		const size_t InputReportLength = 547;
		BytesRead = SDL_hid_read(Context->Handle, Context->BufferDS4, InputReportLength);
		if (BytesRead < 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("hid_api: Failed to read from device (likely disconnected)"));
			InvalidateHandle(Context);
		}
		return;
	}
	
	const size_t InputReportLength = (Context->ConnectionType == Bluetooth) ? 78 : 64;
	if (sizeof(Context->Buffer) < InputReportLength)
	{
		UE_LOG(LogTemp, Warning, TEXT("hid_api: Main buffer is too small for report input."));
		InvalidateHandle(Context);
		return;
	}

	BytesRead = SDL_hid_read(Context->Handle, Context->Buffer, InputReportLength);
	if (BytesRead < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("hid_api: Failed to read from device (likely disconnected)"));
		InvalidateHandle(Context);
	}
}

void FCommonsDeviceInfo::ProcessAudioHapitc(FDeviceContext* Context)
{
	if (!Context || !Context->Handle)
	{
		return;
	}

	constexpr size_t Report = 142;
	int BytesWritten = SDL_hid_write(Context->Handle, Context->BufferAudio, Report);
	if (BytesWritten < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("hid_api: Failed to write audio device"));
	}
}

void FCommonsDeviceInfo::Write(FDeviceContext* Context)
{
	if (!Context || !Context->Handle)
	{
		return;
	}

	const size_t InReportLength = (Context->DeviceType == EDeviceType::DualShock4) ? 32 : 74;
	const size_t OutputReportLength = (Context->ConnectionType == Bluetooth) ? 78 : InReportLength;

	int BytesWritten = SDL_hid_write(Context->Handle, Context->BufferOutput, OutputReportLength);
	if (BytesWritten < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("hid_api: Failed to write to device"));
		InvalidateHandle(Context);
	}
}

void FCommonsDeviceInfo::Detect(TArray<FDeviceContext>& Devices)
{
	Devices.Empty();

	const TSet<uint16> SupportedPIDs = {
		DUALSHOCK4_PID_V1,
		DUALSHOCK4_PID_V2,
		DUALSENSE_PID,
		DUALSENSE_EDGE_PID
	};

	SDL_hid_device_info* Devs = SDL_hid_enumerate(SONY_VENDOR_ID, 0);
	if (!Devs)
	{
		return;
	}
	
	for (SDL_hid_device_info* CurrentDevice = Devs; CurrentDevice != nullptr; CurrentDevice = CurrentDevice->next)
	{
		if (SupportedPIDs.Contains(CurrentDevice->product_id))
		{
			FDeviceContext NewDeviceContext;
			FString PathStr = UTF8_TO_TCHAR(CurrentDevice->path);
			NewDeviceContext.Path = PathStr;
			
			switch (CurrentDevice->product_id)
			{
			case DUALSHOCK4_PID_V1:
			case DUALSHOCK4_PID_V2:
				NewDeviceContext.DeviceType = EDeviceType::DualShock4;
				break;
			case DUALSENSE_EDGE_PID:
				NewDeviceContext.DeviceType = EDeviceType::DualSenseEdge;
				break;
			case DUALSENSE_PID:
			default:
				NewDeviceContext.DeviceType = EDeviceType::DualSense;
				break;
			}
	
			NewDeviceContext.IsConnected = true;
			if (CurrentDevice->interface_number == -1)
			{
				NewDeviceContext.ConnectionType = Bluetooth;
			}
			else
			{
				NewDeviceContext.ConnectionType = Usb;
			}
			NewDeviceContext.Handle = nullptr;
			Devices.Add(NewDeviceContext);
		}
	}
	SDL_hid_free_enumeration(Devs);
}

bool FCommonsDeviceInfo::CreateHandle(FDeviceContext* Context)
{
	if (!Context || Context->Path.IsEmpty())
	{
		return false;
	}
  
	const FTCHARToUTF8 PathConverter(*Context->Path);
	const FPlatformDeviceHandle Handle =  SDL_hid_open_path(PathConverter.Get(), true);
	if (Handle == INVALID_PLATFORM_HANDLE)
	{
		return false;
	}
	
	SDL_hid_set_nonblocking(Handle, 1);
	Context->Handle = Handle;
	return true;
}

void FCommonsDeviceInfo::InvalidateHandle(FDeviceContext* Context)
{
	if (Context && Context->Handle)
	{
		SDL_hid_close(Context->Handle);
		Context->Handle = INVALID_PLATFORM_HANDLE;
		Context->IsConnected = false;

		Context->Path = nullptr;
		memset(Context->Buffer, 0, sizeof(Context->Buffer));
		memset(Context->BufferDS4, 0, sizeof(Context->BufferDS4));
		memset(Context->BufferOutput, 0, sizeof(Context->BufferOutput));
		memset(Context->BufferAudio, 0, sizeof(Context->BufferAudio));
	}
}
#endif
