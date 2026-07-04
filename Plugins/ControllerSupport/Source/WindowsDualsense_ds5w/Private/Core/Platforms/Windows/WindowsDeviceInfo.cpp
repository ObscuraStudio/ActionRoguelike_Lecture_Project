// Copyright (c) 2025 Rafael Valoto/Publisher. All rights reserved.
// Created for: WindowsDualsense_ds5w - Plugin to support DualSense controller on Windows.
// Planned Release Year: 2025

#include "Core/Platforms/Windows/WindowsDeviceInfo.h"
#include "Runtime/ApplicationCore/Public/GenericPlatform/IInputInterface.h"
#include "Runtime/ApplicationCore/Public/GenericPlatform/GenericApplicationMessageHandler.h"
#include <hidsdi.h>
#include <setupapi.h>

void FWindowsDeviceInfo::Detect(TArray<FDeviceContext>& Devices)
{
	GUID HidGuid;
	HidD_GetHidGuid(&HidGuid);

	const HDEVINFO DeviceInfoSet = SetupDiGetClassDevs(&HidGuid, nullptr, nullptr,
	                                                   DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (DeviceInfoSet == INVALID_HANDLE_VALUE)
	{
		UE_LOG(LogTemp, Error, TEXT("HIDManager: Falha ao obter informações dos dispositivos HID."));
		return;
	}

	SP_DEVICE_INTERFACE_DATA DeviceInterfaceData = {};
	DeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

	TMap<int32, FString> DevicePaths;
	for (int32 DeviceIndex = 0; SetupDiEnumDeviceInterfaces(DeviceInfoSet, nullptr, &HidGuid, DeviceIndex,
	                                                        &DeviceInterfaceData); DeviceIndex++)
	{
		DWORD RequiredSize = 0;
		SetupDiGetDeviceInterfaceDetail(DeviceInfoSet, &DeviceInterfaceData, nullptr, 0, &RequiredSize, nullptr);

		const auto DetailDataBuffer = static_cast<PSP_DEVICE_INTERFACE_DETAIL_DATA>(malloc(RequiredSize));
		if (!DetailDataBuffer)
		{
			UE_LOG(LogTemp, Error, TEXT("HIDManager: Failed to allocate memory for device details."));
			continue;
		}

		DetailDataBuffer->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		if (SetupDiGetDeviceInterfaceDetail(DeviceInfoSet, &DeviceInterfaceData, DetailDataBuffer, RequiredSize,
		                                    nullptr, nullptr))
		{
			const HANDLE TempDeviceHandle = CreateFileW(
				DetailDataBuffer->DevicePath,
				GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, NULL, nullptr
			);
			
			if (TempDeviceHandle != INVALID_HANDLE_VALUE)
			{
				HIDD_ATTRIBUTES Attributes = {};
				Attributes.Size = sizeof(HIDD_ATTRIBUTES);

				if (HidD_GetAttributes(TempDeviceHandle, &Attributes))
				{
					if (
						Attributes.VendorID == 0x054C &&
						(
							Attributes.ProductID == 0x0CE6 ||
							Attributes.ProductID == 0x0DF2 ||
							Attributes.ProductID == 0x05C4 ||
							Attributes.ProductID == 0x09CC
						)
					)
					{
						FDeviceContext Context = {};
						WCHAR DeviceProductString[260];
						if (HidD_GetProductString(TempDeviceHandle, DeviceProductString, 260))
						{
							if (!DevicePaths.Contains(DeviceIndex))
							{
								FString PathStr(DetailDataBuffer->DevicePath);
								Context.Path = PathStr;
								DevicePaths.Add(DeviceIndex, PathStr);
								switch (Attributes.ProductID)
								{
									case 0x05C4:
									case 0x09CC:
										Context.DeviceType = DualShock4;
										break;
									case 0x0DF2:
										Context.DeviceType = DualSenseEdge;
										break;
									default: Context.DeviceType = DualSense;
								}

								Context.IsConnected = true;
								Context.ConnectionType = Usb;
								FString DevicePath(Context.Path);
								if (DevicePath.Contains(TEXT("{00001124-0000-1000-8000-00805f9b34fb}")) ||
									DevicePath.Contains(TEXT("bth")) ||
									DevicePath.Contains(TEXT("BTHENUM")))
								{
									Context.ConnectionType = Bluetooth;
									if (!ConfigureBluetoothFeatures(TempDeviceHandle))
									{
										UE_LOG(LogTemp, Warning, TEXT("HIDManager: Failed to configure Bluetooth features."));
									}
								}
								Devices.Add(Context);
							}
						}
						else
						{
							UE_LOG(LogTemp, Error, TEXT("HIDManager: Failed to obtain device path for the DualSense."));
						}
					}
				}
				CloseHandle(TempDeviceHandle);
			}
		}
		free(DetailDataBuffer);
	}

	SetupDiDestroyDeviceInfoList(DeviceInfoSet);
}



void FWindowsDeviceInfo::Read(FDeviceContext* Context)
{
	if (!Context)
	{
		UE_LOG(LogTemp, Error, TEXT("Context nto found!"));
		return;
	}
	
	if (Context->Handle == INVALID_HANDLE_VALUE)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid device handle before attempting to read"));
		return;
	}
	
	if (!Context->IsConnected)
	{
		UE_LOG(LogTemp, Error, TEXT("Dualsense: DeviceContext->Connected, false"));
		return;
	}

	DWORD BytesRead = 0;
	HidD_FlushQueue(Context->Handle);
	
	if (Context->ConnectionType == Bluetooth && Context->DeviceType == EDeviceType::DualShock4)
	{
		constexpr size_t InputReportLength = 547;
		PollTick(Context->Handle, Context->BufferDS4, InputReportLength, BytesRead);
	}
	else
	{
		const size_t InputBufferSize = Context->ConnectionType == Bluetooth ? 78 : 64;
		PollTick(Context->Handle, Context->Buffer, InputBufferSize, BytesRead);
	}
}

void FWindowsDeviceInfo::Write(FDeviceContext* Context)
{
	if (Context->Handle == INVALID_HANDLE_VALUE)
	{
		return;
	}
	
	size_t InReportLength = Context->DeviceType == DualShock4 ? 32 : 74;
	size_t OutputReportLength = Context->ConnectionType == Bluetooth ? 78 : InReportLength;

	DWORD BytesWritten = 0;
	if (!WriteFile(Context->Handle, Context->BufferOutput, OutputReportLength, &BytesWritten, nullptr))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to write output report 0x02/0x31 data to device. report %llu error Code: %d"),
			OutputReportLength, GetLastError());
	}
}

bool FWindowsDeviceInfo::CreateHandle(FDeviceContext* DeviceContext)
{
	const HANDLE DeviceHandle = CreateFileW(
			*DeviceContext->Path,
			GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, NULL, nullptr
		);

	if (DeviceHandle == INVALID_HANDLE_VALUE)
	{
		DeviceContext->Handle = DeviceHandle;
		UE_LOG(LogTemp, Error, TEXT("HIDManager: Failed to open device handle for the DualSense."));
		return false;
	}
	
	DeviceContext->Handle = DeviceHandle;
	return true;
}

void FWindowsDeviceInfo::InvalidateHandle(FDeviceContext* Context)
{
	if (!Context)
	{
		return;
	}

	if (Context->Handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(Context->Handle);
		Context->Handle = INVALID_HANDLE_VALUE;
		Context->IsConnected = false;
		Context->Path = nullptr;

		ZeroMemory(Context->BufferOutput, sizeof(Context->BufferOutput));
		ZeroMemory(Context->BufferAudio, sizeof(Context->BufferAudio));
		ZeroMemory(Context->Buffer, sizeof(Context->Buffer));
		ZeroMemory(Context->BufferDS4, sizeof(Context->BufferDS4));
	}
}

void FWindowsDeviceInfo::InvalidateHandle(HANDLE Handle)
{
	if (Handle != INVALID_PLATFORM_HANDLE)
	{
		CloseHandle(Handle);
		UE_LOG(LogTemp, Log, TEXT("HIDManager: Invalidate Handle."));
	}
}

EPollResult FWindowsDeviceInfo::PollTick(HANDLE Handle, unsigned char* Buffer, int32 Length, DWORD& OutBytesRead)
{
	int32 Err = ERROR_SUCCESS;
	PingOnce(Handle, &Err);
	
	OutBytesRead = 0;
	if (!ReadFile(Handle, Buffer, Length, &OutBytesRead, nullptr))
	{
		return EPollResult::Disconnected;
	}

	return EPollResult::ReadOk;
}

bool FWindowsDeviceInfo::PingOnce(HANDLE Handle, int32* OutLastError)
{
	FILE_STANDARD_INFO Info{};
	if (!GetFileInformationByHandleEx(Handle, FileStandardInfo, &Info, sizeof(Info)))
	{
		if (OutLastError) *OutLastError = GetLastError();
		return false;
	}
	if (OutLastError) *OutLastError = ERROR_SUCCESS;
	return true;
}

void FWindowsDeviceInfo::ProcessAudioHapitc(FDeviceContext* Context)
{
	if (!Context || !Context->Handle)
	{
		return;
	}

	if (Context->Handle == INVALID_HANDLE_VALUE)
	{
		return;
	}
	
	if (Context->ConnectionType != Bluetooth)
	{
		return;
	}
	
	DWORD BytesWritten = 0;
	constexpr size_t BufferSize = 142;
	if (!WriteFile(Context->Handle, Context->BufferAudio, BufferSize, &BytesWritten, nullptr))
	{
		const DWORD Error = GetLastError();
		if (Error != ERROR_IO_PENDING)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to send audio haptics via WriteFile. Error: %d"), Error);
		}
	}
}

bool FWindowsDeviceInfo::ConfigureBluetoothFeatures(HANDLE DeviceHandle)
{
	// Feature Report 0x05 - Enables advanced Bluetooth features
	unsigned char FeatureBuffer[41];
	FMemory::Memzero(FeatureBuffer, sizeof(FeatureBuffer));
	FeatureBuffer[0] = 0x05;
	if (!HidD_GetFeature(DeviceHandle, FeatureBuffer, 41))
	{
		const DWORD Error = GetLastError();
		UE_LOG(LogTemp, Warning, TEXT("HIDManager: Failed to get Feature 0x05. Error: %d"), Error);
		return false;
	}

	return true;
}
