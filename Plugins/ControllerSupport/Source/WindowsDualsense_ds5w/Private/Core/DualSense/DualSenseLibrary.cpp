// Copyright (c) 2025 Rafael Valoto/Publisher. All rights reserved.
// Created for: WindowsDualsense_ds5w - Plugin to support DualSense controller on Windows.
// Planned Release Year: 2025

#include "Core/DualSense/DualSenseLibrary.h"

#include "DeviceManager.h"
#include "Async/Async.h"
#include "Async/TaskGraphInterfaces.h"
#include "InputCoreTypes.h"
#include "Helpers/ValidateHelpers.h"
#include "Core/Interfaces/PlatformHardwareInfoInterface.h"
#include "Core/PlayStationOutputComposer.h"
#include "Core/Algorithms/MadgwickAhrs.h"
#include "Core/Structs/OutputContext.h"

bool UDualSenseLibrary::InitializeLibrary(const FDeviceContext& Context)
{
	HIDDeviceContexts = Context;
	if (HIDDeviceContexts.ConnectionType == Bluetooth)
	{
		FOutputContext* EnableReport = &HIDDeviceContexts.Output;
		// Set flags to enable control over the lightbar, player LEDs
		EnableReport->Feature.FeatureMode = 0x55;
		EnableReport->Lightbar = {0, 0, 222};
		EnableReport->PlayerLed.Brightness = 0x00;
		SendOut();

		FPlatformProcess::Sleep(0.1f);
		HIDDeviceContexts.BufferAudio[0] = 0x32;
		HIDDeviceContexts.BufferAudio[1] = 0x00;
		HIDDeviceContexts.BufferAudio[2] = 0x91;
		HIDDeviceContexts.BufferAudio[3] = 0x07;
		HIDDeviceContexts.BufferAudio[4] = 0xFE;
		HIDDeviceContexts.BufferAudio[5] = 55;
		HIDDeviceContexts.BufferAudio[6] = 55;
		HIDDeviceContexts.BufferAudio[7] = 15;
		HIDDeviceContexts.BufferAudio[8] = 50;
		HIDDeviceContexts.BufferAudio[9] = 50;
	}

	StopAll();
	return true;
}

void UDualSenseLibrary::ShutdownLibrary()
{
	ButtonStates.Reset();
	IPlatformHardwareInfoInterface::Get().InvalidateHandle(&HIDDeviceContexts);
}

bool UDualSenseLibrary::IsConnected() { return HIDDeviceContexts.IsConnected; }

void UDualSenseLibrary::SendOut()
{
	if (!HIDDeviceContexts.IsConnected) { return; }

	FPlayStationOutputComposer::OutputDualSense(&HIDDeviceContexts);
}

void UDualSenseLibrary::Settings(const FDualSenseFeatureReport& Settings)
{
	FOutputContext* HidOutput = &HIDDeviceContexts.Output;
	if (Settings.VibrationMode == EDualSenseDeviceFeatureReport::Off) { HidOutput->Feature.VibrationMode = 0xFC; }

	HidOutput->Feature.SoftRumbleReduce = static_cast<uint8>(Settings.SoftRumbleReduce);
	HidOutput->Feature.TriggerSoftnessLevel = static_cast<uint8>(Settings.TriggerSoftnessLevel);
	HidOutput->Audio.MicStatus = static_cast<uint8>(Settings.MicStatus);
	HidOutput->Audio.MicVolume = static_cast<uint8>(Settings.MicVolume);
	HidOutput->Audio.HeadsetVolume = static_cast<uint8>(Settings.AudioVolume);
	HidOutput->Audio.SpeakerVolume = static_cast<uint8>(Settings.AudioVolume);
	HidOutput->Audio.Mode = 0x08;
	if (Settings.AudioHeadset == EDualSenseAudioFeatureReport::On && Settings.AudioSpeaker ==
		EDualSenseAudioFeatureReport::Off) { HidOutput->Audio.Mode = 0x31; }

	if (Settings.AudioHeadset == EDualSenseAudioFeatureReport::Off && Settings.AudioSpeaker ==
		EDualSenseAudioFeatureReport::On) { HidOutput->Audio.Mode = 0x21; }
	SendOut();
}

void UDualSenseLibrary::CheckButtonInput(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler,
                                         const FPlatformUserId UserId, const FInputDeviceId InputDeviceId,
                                         const FName ButtonName, const bool IsButtonPressed)
{
	const bool PreviousState = ButtonStates.Contains(ButtonName) ? ButtonStates[ButtonName] : false;
	if (IsButtonPressed && !PreviousState)
	{
		SetControllerEvents(true);
		InMessageHandler.Get().OnControllerButtonPressed(ButtonName, UserId, InputDeviceId, false);
	}

	if (!IsButtonPressed && PreviousState)
	{
		SetControllerEvents(true);
		InMessageHandler.Get().OnControllerButtonReleased(ButtonName, UserId, InputDeviceId, false);
	}

	ButtonStates.Add(ButtonName, IsButtonPressed);
}

void UDualSenseLibrary::UpdateInput(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler,
                                    const FPlatformUserId UserId, const FInputDeviceId InputDeviceId, float Delta)
{
	FDeviceContext* Context = &HIDDeviceContexts;
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [NewContext = MoveTemp(Context)]()
	{
		IPlatformHardwareInfoInterface::Get().Read(NewContext);
	});

	// FValidateHelpers::PrintBufferAsHex(HIDDeviceContexts.Buffer, 78, TEXT(""));
	const size_t Padding = HIDDeviceContexts.ConnectionType == Bluetooth ? 2 : 1;
	const unsigned char* HIDInput = &HIDDeviceContexts.Buffer[Padding];

	const float LeftAnalogX = static_cast<char>(static_cast<short>(HIDInput[0x00] - 128));
	const float LeftAnalogY = static_cast<char>(static_cast<short>(HIDInput[0x01] - 127) * -1);
	InMessageHandler.Get().OnControllerAnalog(FGamepadKeyNames::LeftAnalogX, UserId, InputDeviceId,
	                                          LeftAnalogX / 128.0f);
	InMessageHandler.Get().OnControllerAnalog(FGamepadKeyNames::LeftAnalogY, UserId, InputDeviceId,
	                                          LeftAnalogY / 128.0f);

	const float RightAnalogX = static_cast<char>(static_cast<short>(HIDInput[0x02] - 128));
	const float RightAnalogY = static_cast<char>(static_cast<short>(HIDInput[0x03] - 127) * -1);
	InMessageHandler.Get().OnControllerAnalog(FGamepadKeyNames::RightAnalogX, UserId, InputDeviceId,
	                                          RightAnalogX / 128.0f);
	InMessageHandler.Get().OnControllerAnalog(FGamepadKeyNames::RightAnalogY, UserId, InputDeviceId,
	                                          RightAnalogY / 128.0f);

	const float TriggerL = HIDInput[0x04] / 256.0f;
	const float TriggerR = HIDInput[0x05] / 256.0f;
	InMessageHandler.Get().OnControllerAnalog(FGamepadKeyNames::LeftTriggerAnalog, UserId, InputDeviceId, TriggerL);
	InMessageHandler.Get().OnControllerAnalog(FGamepadKeyNames::RightTriggerAnalog, UserId, InputDeviceId, TriggerR);

	uint8_t ButtonsMask = HIDInput[0x07] & 0xF0;
	const bool bCross = ButtonsMask & BTN_CROSS;
	const bool bSquare = ButtonsMask & BTN_SQUARE;
	const bool bCircle = ButtonsMask & BTN_CIRCLE;
	const bool bTriangle = ButtonsMask & BTN_TRIANGLE;

	CheckButtonInput(InMessageHandler, UserId, InputDeviceId, FGamepadKeyNames::FaceButtonBottom, bCross);
	CheckButtonInput(InMessageHandler, UserId, InputDeviceId, FGamepadKeyNames::FaceButtonLeft, bSquare);
	CheckButtonInput(InMessageHandler, UserId, InputDeviceId, FGamepadKeyNames::FaceButtonRight, bCircle);
	CheckButtonInput(InMessageHandler, UserId, InputDeviceId, FGamepadKeyNames::FaceButtonTop, bTriangle);

	CheckButtonInput(InMessageHandler, UserId, InputDeviceId, FGamepadKeyNames::RightStickUp,
	                 RightAnalogY / 128.0f > AnalogDeadZone);
	CheckButtonInput(InMessageHandler, UserId, InputDeviceId, FGamepadKeyNames::RightStickDown,
	                 RightAnalogY / 128.0f < -AnalogDeadZone);
	CheckButtonInput(InMessageHandler, UserId, InputDeviceId, FGamepadKeyNames::RightStickLeft,
	                 RightAnalogX / 128.0f < -AnalogDeadZone);
	CheckButtonInput(InMessageHandler, UserId, InputDeviceId, FGamepadKeyNames::RightStickRight,
	                 RightAnalogX / 128.0f > AnalogDeadZone);

	CheckButtonInput(InMessageHandler, UserId, InputDeviceId, FGamepadKeyNames::LeftStickUp,
	                 LeftAnalogY / 128.0f > AnalogDeadZone);
	CheckButtonInput(InMessageHandler, UserId, InputDeviceId, FGamepadKeyNames::LeftStickDown,
	                 LeftAnalogY / 128.0f < -AnalogDeadZone);
	CheckButtonInput(InMessageHandler, UserId, InputDeviceId, FGamepadKeyNames::LeftStickLeft,
	                 LeftAnalogX / 128.0f < -AnalogDeadZone);
	CheckButtonInput(InMessageHandler, UserId, InputDeviceId, FGamepadKeyNames::LeftStickRight,
	                 LeftAnalogX / 128.0f > AnalogDeadZone);

	switch (HIDInput[0x07] & 0x0F)
	{
	case 0x0:
		ButtonsMask |= BTN_DPAD_UP;
		break;
	case 0x4:
		ButtonsMask |= BTN_DPAD_DOWN;
		break;
	case 0x6:
		ButtonsMask |= BTN_DPAD_LEFT;
		break;
	case 0x2:
		ButtonsMask |= BTN_DPAD_RIGHT;
		break;
	case 0x5:
		ButtonsMask |= BTN_DPAD_LEFT | BTN_DPAD_DOWN;
		break;
	case 0x7:
		ButtonsMask |= BTN_DPAD_LEFT | BTN_DPAD_UP;
		break;
	case 0x1:
		ButtonsMask |= BTN_DPAD_RIGHT | BTN_DPAD_UP;
		break;
	case 0x3:
		ButtonsMask |= BTN_DPAD_RIGHT | BTN_DPAD_DOWN;
		break;
	default: ;
	}
	const bool bDPadLeft = ButtonsMask & BTN_DPAD_LEFT;
	const bool bDPadDown = ButtonsMask & BTN_DPAD_DOWN;
	const bool bDPadRight = ButtonsMask & BTN_DPAD_RIGHT;
	const bool bDPadUp = ButtonsMask & BTN_DPAD_UP;

	CheckButtonInput(InMessageHandler, UserId, InputDeviceId, FGamepadKeyNames::DPadUp, bDPadUp);
	CheckButtonInput(InMessageHandler, UserId, InputDeviceId, FGamepadKeyNames::DPadDown, bDPadDown);
	CheckButtonInput(InMessageHandler, UserId, InputDeviceId, FGamepadKeyNames::DPadLeft, bDPadLeft);
	CheckButtonInput(InMessageHandler, UserId, InputDeviceId, FGamepadKeyNames::DPadRight, bDPadRight);

	// Shoulders
	const bool bLeftShoulder = HIDInput[0x08] & BTN_LEFT_SHOLDER;
	const bool bRightShoulder = HIDInput[0x08] & BTN_RIGHT_SHOLDER;

	CheckButtonInput(InMessageHandler, UserId, InputDeviceId, FGamepadKeyNames::LeftShoulder, bLeftShoulder);
	CheckButtonInput(InMessageHandler, UserId, InputDeviceId, FGamepadKeyNames::RightShoulder, bRightShoulder);

	// Push Stick
	const bool PushLeftStick = HIDInput[0x08] & BTN_LEFT_STICK;
	const bool PushRightStick = HIDInput[0x08] & BTN_RIGHT_STICK;
	CheckButtonInput(InMessageHandler, UserId, InputDeviceId, FName("PS_PushLeftStick"), PushLeftStick);
	CheckButtonInput(InMessageHandler, UserId, InputDeviceId, FName("PS_PushRightStick"), PushRightStick);

	// mapped urenal native gamepad Push Stick
	CheckButtonInput(InMessageHandler, UserId, InputDeviceId, FGamepadKeyNames::LeftThumb, PushLeftStick);
	CheckButtonInput(InMessageHandler, UserId, InputDeviceId, FGamepadKeyNames::RightThumb, PushRightStick);

	// Function & Special Actions
	const bool Playstation = HIDInput[0x09] & BTN_PLAYSTATION_LOGO;
	const bool TouchPad = HIDInput[0x09] & BTN_PAD_BUTTON;
	const bool Mic = HIDInput[0x09] & BTN_MIC_BUTTON;
	const bool bFn1 = HIDInput[0x09] & BTN_FN1;
	const bool bFn2 = HIDInput[0x09] & BTN_FN2;
	const bool bPaddleLeft = HIDInput[0x09] & BTN_PADDLE_LEFT;
	const bool bPaddleRight = HIDInput[0x09] & BTN_PADDLE_RIGHT;

	CheckButtonInput(InMessageHandler, UserId, InputDeviceId, FName("PS_Mic"), Mic);
	CheckButtonInput(InMessageHandler, UserId, InputDeviceId, FName("PS_TouchButtom"), TouchPad);
	CheckButtonInput(InMessageHandler, UserId, InputDeviceId, FName("PS_Button"), Playstation);
	CheckButtonInput(InMessageHandler, UserId, InputDeviceId, FName("PS_FunctionL"), bFn1);
	CheckButtonInput(InMessageHandler, UserId, InputDeviceId, FName("PS_FunctionR"), bFn2);
	CheckButtonInput(InMessageHandler, UserId, InputDeviceId, FName("PS_PaddleL"), bPaddleLeft);
	CheckButtonInput(InMessageHandler, UserId, InputDeviceId, FName("PS_PaddleR"), bPaddleRight);

	const bool Start = HIDInput[0x08] & BTN_START;
	const bool Select = HIDInput[0x08] & BTN_SELECT;
	CheckButtonInput(InMessageHandler, UserId, InputDeviceId, FName("PS_Menu"), Start);
	CheckButtonInput(InMessageHandler, UserId, InputDeviceId, FName("PS_Share"), Select);

	// mapped urenal native gamepad Start and Select
	CheckButtonInput(InMessageHandler, UserId, InputDeviceId, FGamepadKeyNames::SpecialRight, Start);
	CheckButtonInput(InMessageHandler, UserId, InputDeviceId, FGamepadKeyNames::SpecialLeft, Select);


	const bool bLeftTriggerThreshold = HIDInput[0x08] & BTN_LEFT_TRIGGER;
	const bool bRightTriggerThreshold = HIDInput[0x08] & BTN_RIGHT_TRIGGER;
	CheckButtonInput(InMessageHandler, UserId, InputDeviceId, FGamepadKeyNames::LeftTriggerThreshold,
	                 bLeftTriggerThreshold);
	CheckButtonInput(InMessageHandler, UserId, InputDeviceId, FGamepadKeyNames::RightTriggerThreshold,
	                 bRightTriggerThreshold);
	if (bEnableTouch)
	{
		FTouchPoint1 Touch;
		const int32 Touchpad1Raw = *reinterpret_cast<const int32*>(&HIDInput[0x20]);
		Touch.Y = (Touchpad1Raw & 0xFFF00000) >> 20;
		Touch.X = (Touchpad1Raw & 0x000FFF00) >> 8;
		Touch.Down = (Touchpad1Raw & (1 << 7)) == 0;
		Touch.Id = (Touchpad1Raw & 127) % 10;

		bool bIsTouchDown = Touch.Down;
		if (bIsTouchDown) // pressed
		{
			if (!bWasTouch1Down)
			{
				const FVector2D TouchVectorStart = FVector2D(Touch.X, Touch.Y);
				InMessageHandler->OnTouchStarted(
					nullptr,
					TouchVectorStart,
					1.0f,
					Touch.Id,
					UserId,
					InputDeviceId
				);
			}
			else
			{
				const FVector2D TouchVector = FVector2D(Touch.X, Touch.Y);
				InMessageHandler->OnTouchMoved(TouchVector, 1.0f, Touch.Id, UserId, InputDeviceId);
			}
		}
		else if (!bIsTouchDown && bWasTouch1Down)
		{
			const FVector2D TouchVectorEnded = FVector2D(Touch.X, Touch.Y);
			InMessageHandler->OnTouchEnded(
				TouchVectorEnded,
				Touch.Id,
				UserId,
				InputDeviceId
			);
		}

		bWasTouch1Down = bIsTouchDown;

		FTouchPoint2 Touch2;
		const int32 Touchpad2Raw = *reinterpret_cast<const int32*>(&HIDInput[0x24]);
		Touch2.Y = (Touchpad2Raw & 0xFFF00000) >> 20;
		Touch2.X = (Touchpad2Raw & 0x000FFF00) >> 8;
		Touch2.Down = (Touchpad2Raw & (1 << 7)) == 0;
		Touch2.Id = (Touchpad2Raw & 127) % 10;

		bool bIsTouch2Down = Touch2.Down;
		if (bIsTouch2Down)
		{
			if (!bWasTouch2Down)
			{
				const FVector2D Touch2VectorStart = FVector2D(Touch2.X, Touch2.Y);
				InMessageHandler->OnTouchStarted(nullptr, Touch2VectorStart, 1.0f, Touch2.Id, UserId, InputDeviceId);
			}
			else
			{
				const FVector2D Touch2Vector = FVector2D(Touch2.X, Touch2.Y);
				InMessageHandler->OnTouchMoved(Touch2Vector, 1.0f, Touch2.Id, UserId, InputDeviceId);
			}
		}
		else if (!bIsTouch2Down && bWasTouch2Down)
		{
			const FVector2D Touch2VectorEnded = FVector2D(Touch2.X, Touch2.Y);
			InMessageHandler->OnTouchEnded(
				Touch2VectorEnded,
				Touch2.Id,
				UserId,
				InputDeviceId
			);
		}

		bWasTouch2Down = bIsTouch2Down;
	}

	if (bEnableAccelerometerAndGyroscope)
	{
		FGyro Gyro;
		Gyro.X = static_cast<int16_t>((HIDInput[16]) | (HIDInput[17] << 8));
		Gyro.Y = static_cast<int16_t>((HIDInput[18]) | (HIDInput[19] << 8));
		Gyro.Z = static_cast<int16_t>((HIDInput[20]) | (HIDInput[21] << 8));

		FAccelerometer Acc;
		Acc.X = static_cast<int16_t>((HIDInput[22]) | (HIDInput[23] << 8));
		Acc.Y = static_cast<int16_t>((HIDInput[24]) | (HIDInput[25] << 8));
		Acc.Z = static_cast<int16_t>((HIDInput[26]) | (HIDInput[27] << 8));

		if (bIsCalibrating)
		{
			AccumulatedGyro.X += Gyro.X;
			AccumulatedGyro.Y += Gyro.Y;
			AccumulatedGyro.Z += Gyro.Z;

			AccumulatedAccel.X += Acc.X;
			AccumulatedAccel.Y += Acc.Y;
			AccumulatedAccel.Z += Acc.Z;


			Bounds.Gyro_X_Bounds.X = FMath::Min(Bounds.Gyro_X_Bounds.X, Gyro.X);
			Bounds.Gyro_X_Bounds.Y = FMath::Max(Bounds.Gyro_X_Bounds.Y, Gyro.X);

			Bounds.Gyro_Y_Bounds.X = FMath::Min(Bounds.Gyro_Y_Bounds.X, Gyro.Y);
			Bounds.Gyro_Y_Bounds.Y = FMath::Max(Bounds.Gyro_Y_Bounds.Y, Gyro.Y);

			Bounds.Gyro_Z_Bounds.X = FMath::Min(Bounds.Gyro_Z_Bounds.X, Gyro.Z);
			Bounds.Gyro_Z_Bounds.Y = FMath::Max(Bounds.Gyro_Z_Bounds.Y, Gyro.Z);

			Bounds.Accel_X_Bounds.X = FMath::Min(Bounds.Accel_X_Bounds.X, Acc.X);
			Bounds.Accel_X_Bounds.Y = FMath::Max(Bounds.Accel_X_Bounds.Y, Acc.X);

			Bounds.Accel_Y_Bounds.X = FMath::Min(Bounds.Accel_Y_Bounds.X, Acc.Y);
			Bounds.Accel_Y_Bounds.Y = FMath::Max(Bounds.Accel_Y_Bounds.Y, Acc.Y);

			Bounds.Accel_Z_Bounds.X = FMath::Min(Bounds.Accel_Z_Bounds.X, Acc.Z);
			Bounds.Accel_Z_Bounds.Y = FMath::Max(Bounds.Accel_Z_Bounds.Y, Acc.Z);

			CalibrationSampleCount++;
		}

		if (bHasMotionSensorBaseline)
		{
			Gyro.X -= GyroBaseline.X;
			Gyro.Y -= GyroBaseline.Y;
			Gyro.Z -= GyroBaseline.Z;

			float FinalGyroValueX = 0.0f;
			if (FMath::Abs(Gyro.X) > (Bounds.Gyro_X_Bounds.Y - Bounds.Gyro_X_Bounds.X) * SensorsDeadZone)
			{
				FinalGyroValueX = Gyro.X;
			}

			float FinalGyroValueY = 0.0f;
			if (FMath::Abs(Gyro.Y) > (Bounds.Gyro_Y_Bounds.Y - Bounds.Gyro_Y_Bounds.X) * SensorsDeadZone)
			{
				FinalGyroValueY = Gyro.Y;
			}

			float FinalGyroValueZ = 0.0f;
			if (FMath::Abs(Gyro.Z) > (Bounds.Gyro_Z_Bounds.Y - Bounds.Gyro_Z_Bounds.X) * SensorsDeadZone)
			{
				FinalGyroValueZ = Gyro.Z;
			}

			Acc.X -= AccelBaseline.X;
			Acc.Y -= AccelBaseline.Y;
			Acc.Z -= AccelBaseline.Z;

			float FinalAccelValueX = 0.0f;
			if (FMath::Abs(Acc.X) > (Bounds.Accel_X_Bounds.Y - Bounds.Accel_X_Bounds.X) * SensorsDeadZone)
			{
				FinalAccelValueX = Acc.X;
			}

			float FinalAccelValueY = 0.0f;
			if (FMath::Abs(Acc.Y) > (Bounds.Accel_Y_Bounds.Y - Bounds.Accel_Y_Bounds.X) * SensorsDeadZone)
			{
				FinalAccelValueY = Acc.Y;
			}

			float FinalAccelValueZ = 0.0f;
			if (FMath::Abs(Acc.Z) > (Bounds.Accel_Z_Bounds.Y - Bounds.Accel_Z_Bounds.X) *
				SensorsDeadZone)
			{
				FinalAccelValueZ = Acc.Z;
			}

			Gyro.X = FinalGyroValueX;
			Gyro.Y = FinalGyroValueY;
			Gyro.Z = FinalGyroValueZ;

			Acc.X = FinalAccelValueX;
			Acc.Y = FinalAccelValueY;
			Acc.Z = FinalAccelValueZ;
		}

		// ---------- REPLACED: Complementary Slerp fusion  ----------
		// We now use the Madgwick AHRS (IMU-only) and feed it with values
		// converted from raw counts to SI using the official DS constants.
		// The Madgwick instance and initialization are static locals so that
		// we don't need to change the class header right away.

		static FMadgwickAhrs MadgwickFilter(200.0f, 0.08f);
		static bool bMadgwickInitialized = false;

		// Official PlayStation DualSense scaling constants (from kernel driver)
		constexpr float DS_ACC_RES_PER_G = 8192.0f; // counts per 1 g
		constexpr float DS_GYRO_RES_PER_DEG_S = 1024.0f; // counts per 1 deg/s
		constexpr float G_TO_MS2 = 9.80665f;
		constexpr float DEG2RAD = 3.14159265358979323846f / 180.0f;

		// Convert gyro raw (counts) -> deg/s -> rad/s
		float gx_dps = static_cast<float>(Gyro.X) / DS_GYRO_RES_PER_DEG_S;
		float gy_dps = static_cast<float>(Gyro.Y) / DS_GYRO_RES_PER_DEG_S;
		float gz_dps = static_cast<float>(Gyro.Z) / DS_GYRO_RES_PER_DEG_S;

		float gx = gx_dps * DEG2RAD;
		float gy = gy_dps * DEG2RAD;
		float gz = gz_dps * DEG2RAD;

		float ax_g = static_cast<float>(Acc.X) / DS_ACC_RES_PER_G;
		float ay_g = static_cast<float>(Acc.Y) / DS_ACC_RES_PER_G;
		float az_g = static_cast<float>(Acc.Z) / DS_ACC_RES_PER_G;

		float ax = ax_g * G_TO_MS2;
		float ay = ay_g * G_TO_MS2;
		float az = az_g * G_TO_MS2;

		if (!bMadgwickInitialized)
		{
			const float safeDt = FMath::Max(Delta, 0.001f);
			MadgwickFilter.SetSampleFreq(1.0f / safeDt);
			MadgwickFilter.SetBeta(0.08f);
			bMadgwickInitialized = true;
		}

		if (bIsResetGyroscope)
		{
			MadgwickFilter.Reset();
			bIsResetGyroscope = false;
		}

		// Update Madgwick filter (IMU-only)
		MadgwickFilter.UpdateImu(gx, gy, -gz, ax, ay, -az, Delta);

		// Get quaternion directly to avoid Gimbal Lock
		float qw, qx, qy, qz;
		MadgwickFilter.GetQuaternion(qw, qx, qy, qz);

		// Create Unreal quaternion and extract Euler angles
		// Note: FQuat constructor is (X, Y, Z, W)
		const FQuat SensorQuat(qx, qy, qz, qw);
		const FRotator ControlRotation = SensorQuat.Rotator();

		// Compose Tilt vector using same layout your code used before (Pitch, Yaw, Roll) in degrees
		const FVector Tilt = FVector(ControlRotation.Pitch,
		                             ControlRotation.Yaw,
		                             ControlRotation.Roll);

		// Keep the same output vectors you already used elsewhere
		const FVector Gyroscope = FVector(Gyro.X, Gyro.Z, Gyro.Y);
		const FVector Accelerometer = FVector(Acc.X, Acc.Z, Acc.Y);

		FVector Accel_MS2 = FVector(ax, az, ay);
		const float GravityMagnitude = Accel_MS2.Size();
		FVector Gravity = (GravityMagnitude > KINDA_SMALL_NUMBER)
			                  ? (Accel_MS2 / GravityMagnitude) * G_TO_MS2
			                  : FVector::ZeroVector;

		InMessageHandler.Get().OnMotionDetected(Tilt, Gyroscope, Gravity, Accelerometer, UserId, InputDeviceId);
	}

	SetHasPhoneConnected(HIDInput[0x35] & 0x01);
	SetLevelBattery(((HIDInput[0x34] & 0x0F) / 10.0) * 100, (HIDInput[0x35] & 0x00), (HIDInput[0x36] & 0x20));
}

void UDualSenseLibrary::ResetGyroOrientation()
{
	bIsResetGyroscope = true;
}

void UDualSenseLibrary::SetVibration(const FForceFeedbackValues& Vibration)
{
	FOutputContext* HidOutput = &HIDDeviceContexts.Output;
	const float LeftRumble = FMath::Max(Vibration.LeftLarge, Vibration.LeftSmall);
	const float RightRumble = FMath::Max(Vibration.RightLarge, Vibration.RightSmall);

	const unsigned char OutputLeft = static_cast<unsigned char>(FValidateHelpers::To255(LeftRumble));
	const unsigned char OutputRight = static_cast<unsigned char>(FValidateHelpers::To255(RightRumble));
	if (HidOutput->Rumbles.Left != OutputLeft || HidOutput->Rumbles.Right != OutputRight)
	{
		HidOutput->Rumbles = {OutputLeft, OutputRight};
		SendOut();
	}
}

void UDualSenseLibrary::SetHapticFeedback(int32 Hand, const FHapticFeedbackValues* Values)
{
	FOutputContext* HidOutput = &HIDDeviceContexts.Output;
	if (Hand == static_cast<int32>(EControllerHand::Left) || Hand == static_cast<int32>(EControllerHand::AnyHand))
	{
		HidOutput->LeftTrigger.Frequency = FValidateHelpers::To255(Values->Frequency);
	}

	if (Hand == static_cast<int32>(EControllerHand::Right) || Hand == static_cast<int32>(EControllerHand::AnyHand))
	{
		HidOutput->RightTrigger.Frequency = FValidateHelpers::To255(Values->Frequency);
	}

	SendOut();
}

void UDualSenseLibrary::SetTriggers(const FInputDeviceProperty* Values)
{
	FOutputContext* HidOutput = &HIDDeviceContexts.Output;
	if (Values->Name == FName("InputDeviceTriggerResistance"))
	{
		const FInputDeviceTriggerResistanceProperty* Resistance = static_cast<const
			FInputDeviceTriggerResistanceProperty*>(Values);

		const uint8_t Start = Resistance->StartPosition;
		const uint8_t End = Resistance->EndPosition;
		const float StartStr = static_cast<float>(Resistance->StartStrengh);
		const float EndStr = static_cast<float>(Resistance->EndStrengh);

		constexpr int NumZones = 10;
		uint8_t Strengths[NumZones] = {0};
		if (End > Start)
		{
			for (int i = Start; i <= End && i < NumZones; ++i)
			{
				const float Alpha = (End == Start) ? 0.0f : (i - Start) / (End - Start);
				Strengths[i] = static_cast<uint8>(StartStr + Alpha * (EndStr - StartStr));
			}
		}


		int32 ActiveZones = 0;
		int64 StrengthZones = 0;
		for (int i = 0; i < 10; i++)
		{
			if (Strengths[i] > 0)
			{
				const uint64_t StrengthValue = static_cast<uint64_t>((Strengths[i] - 1) & 0x07);
				StrengthZones |= static_cast<int64>(StrengthValue << (3 * i));
				ActiveZones |= (1 << i);
			}
		}

		if (
			Resistance->AffectedTriggers == EInputDeviceTriggerMask::Left ||
			Resistance->AffectedTriggers == EInputDeviceTriggerMask::All
		)
		{
			HidOutput->LeftTrigger.Mode = 0x02;
			HidOutput->LeftTrigger.Strengths.ActiveZones = ActiveZones;
			HidOutput->LeftTrigger.Strengths.StrengthZones = StrengthZones;
		}

		if (
			Resistance->AffectedTriggers == EInputDeviceTriggerMask::Right ||
			Resistance->AffectedTriggers == EInputDeviceTriggerMask::All
		)
		{
			HidOutput->RightTrigger.Mode = 0x02;
			HidOutput->RightTrigger.Strengths.ActiveZones = ActiveZones;
			HidOutput->RightTrigger.Strengths.StrengthZones = StrengthZones;
		}
	}

	SendOut();
}

void UDualSenseLibrary::SetAutomaticGun(int32 BeginStrength, int32 MiddleStrength, int32 EndStrength,
                                        const EControllerHand& Hand, bool KeepEffect, float Frequency)
{
	HIDDeviceContexts.bOverrideTriggerBytes = false;
	FOutputContext* HidOutput = &HIDDeviceContexts.Output;
	if (Hand == EControllerHand::Left || Hand == EControllerHand::AnyHand)
	{
		HidOutput->LeftTrigger.Mode = 0x26;
		HidOutput->LeftTrigger.Strengths.Compose[0] = 0xe8;
		HidOutput->LeftTrigger.Strengths.Compose[1] = EndStrength > 0 ? 0x07 : 0x08;
		HidOutput->LeftTrigger.Strengths.Compose[2] = 0x00;
		HidOutput->LeftTrigger.Strengths.Compose[3] = FValidateHelpers::To255(MiddleStrength, 10);
		HidOutput->LeftTrigger.Strengths.Compose[4] = FValidateHelpers::To255(EndStrength, 10);
		HidOutput->LeftTrigger.Strengths.Compose[5] = 0x2f;
		HidOutput->LeftTrigger.Strengths.Compose[9] = Frequency;
	}

	if (Hand == EControllerHand::Right || Hand == EControllerHand::AnyHand)
	{
		HidOutput->RightTrigger.Mode = 0x26;
		HidOutput->RightTrigger.Strengths.Compose[0] = 0xe8;
		HidOutput->RightTrigger.Strengths.Compose[1] = EndStrength > 0 ? 0x07 : 0x08;
		HidOutput->RightTrigger.Strengths.Compose[2] = 0x00;
		HidOutput->RightTrigger.Strengths.Compose[3] = FValidateHelpers::To255(MiddleStrength, 10);
		HidOutput->RightTrigger.Strengths.Compose[4] = FValidateHelpers::To255(EndStrength, 10);
		HidOutput->RightTrigger.Strengths.Compose[5] = 0x2f;
		HidOutput->RightTrigger.Strengths.Compose[9] = Frequency;
	}

	SendOut();
}

void UDualSenseLibrary::SetGameCube(const EControllerHand& Hand)
{
	HIDDeviceContexts.bOverrideTriggerBytes = false;
	FOutputContext* HidOutput = &HIDDeviceContexts.Output;
	if (Hand == EControllerHand::Left || Hand == EControllerHand::AnyHand)
	{
		HidOutput->LeftTrigger.Mode = 0x02;
		HidOutput->LeftTrigger.Strengths.Compose[0] = 0x90;
		HidOutput->LeftTrigger.Strengths.Compose[1] = 0x0a;
		HidOutput->LeftTrigger.Strengths.Compose[2] = 0xff;
	}

	if (Hand == EControllerHand::Right || Hand == EControllerHand::AnyHand)
	{
		HidOutput->RightTrigger.Mode = 0x02;
		HidOutput->RightTrigger.Strengths.Compose[0] = 0x90;
		HidOutput->RightTrigger.Strengths.Compose[1] = 0x0a;
		HidOutput->RightTrigger.Strengths.Compose[2] = 0xff;
	}

	SendOut();
}

void UDualSenseLibrary::SetContinuousResistance(int32 StartPosition, int32 Strength, const EControllerHand& Hand)
{
	HIDDeviceContexts.bOverrideTriggerBytes = false;
	FOutputContext* HidOutput = &HIDDeviceContexts.Output;
	if (Hand == EControllerHand::Left || Hand == EControllerHand::AnyHand)
	{
		HidOutput->LeftTrigger.Mode = 0x01;
		HidOutput->LeftTrigger.Strengths.ActiveZones = FValidateHelpers::To255(StartPosition, 8);
		HidOutput->LeftTrigger.Strengths.StrengthZones = FValidateHelpers::To255(Strength, 9);
	}

	if (Hand == EControllerHand::Right || Hand == EControllerHand::AnyHand)
	{
		HidOutput->RightTrigger.Mode = 0x01;
		HidOutput->RightTrigger.Strengths.ActiveZones = FValidateHelpers::To255(StartPosition, 8);
		HidOutput->RightTrigger.Strengths.StrengthZones = FValidateHelpers::To255(Strength, 9);
	}

	SendOut();
}

void UDualSenseLibrary::SetResistance(int32 BeginStrength, int32 MiddleStrength, int32 EndStrength,
                                      const EControllerHand& Hand)
{
	FOutputContext* HidOutput = &HIDDeviceContexts.Output;

	if (Hand == EControllerHand::Left || Hand == EControllerHand::AnyHand)
	{
		HidOutput->LeftTrigger.Mode = 0x21;
		HidOutput->LeftTrigger.Strengths.Compose[0] = FValidateHelpers::To255(BeginStrength, 9);
		HidOutput->LeftTrigger.Strengths.Compose[1] = 0x02;
		HidOutput->LeftTrigger.Strengths.Compose[2] = FValidateHelpers::To255(MiddleStrength, 9);
		HidOutput->LeftTrigger.Strengths.Compose[3] = FValidateHelpers::To255(EndStrength, 7);
	}

	if (Hand == EControllerHand::Right || Hand == EControllerHand::AnyHand)
	{
		HidOutput->RightTrigger.Mode = 0x21;
		HidOutput->RightTrigger.Strengths.Compose[0] = FValidateHelpers::To255(BeginStrength, 9);
		HidOutput->RightTrigger.Strengths.Compose[1] = 0x02;
		HidOutput->RightTrigger.Strengths.Compose[2] = FValidateHelpers::To255(MiddleStrength, 9);
		HidOutput->RightTrigger.Strengths.Compose[3] = FValidateHelpers::To255(EndStrength, 7);
	}

	SendOut();
}

void UDualSenseLibrary::SetWeapon(int32 StartPosition, int32 EndPosition, int32 Strength,
                                  const EControllerHand& Hand)
{
	const uint32_t ActiveZones = (1 << StartPosition) | (1 << EndPosition);
	FOutputContext* HidOutput = &HIDDeviceContexts.Output;
	if (Hand == EControllerHand::Left || Hand == EControllerHand::AnyHand)
	{
		HidOutput->LeftTrigger.Mode = 0x25;
		HidOutput->LeftTrigger.Strengths.ActiveZones = ActiveZones;
		HidOutput->LeftTrigger.Strengths.StrengthZones = FValidateHelpers::To255(Strength);
	}

	if (Hand == EControllerHand::Right || Hand == EControllerHand::AnyHand)
	{
		HidOutput->RightTrigger.Mode = 0x25;
		HidOutput->RightTrigger.Strengths.ActiveZones = ActiveZones;
		HidOutput->RightTrigger.Strengths.StrengthZones = FValidateHelpers::To255(Strength);
	}

	SendOut();
}

void UDualSenseLibrary::SetGalloping(int32 StartPosition, int32 EndPosition, int32 FirstFoot, int32 SecondFoot,
                                     float Frequency, const EControllerHand& Hand)
{
	HIDDeviceContexts.bOverrideTriggerBytes = false;
	FOutputContext* HidOutput = &HIDDeviceContexts.Output;

	const uint8 FirstFootNib = static_cast<uint8>(
		FMath::Clamp(FMath::RoundToInt((FirstFoot / 8.0f) * 15.0f), 1, 15));
	const uint8 SecondFootNib = static_cast<uint8>(
		FMath::Clamp(FMath::RoundToInt((SecondFoot / 8.0f) * 15.0f), 1, 15));
	const uint16 PositionMask = (1 << StartPosition) | (1 << EndPosition);
	if (Hand == EControllerHand::Left || Hand == EControllerHand::AnyHand)
	{
		HidOutput->LeftTrigger.Mode = 0x23;
		HidOutput->LeftTrigger.Strengths.Compose[0] = PositionMask & 0xFF;
		HidOutput->LeftTrigger.Strengths.Compose[1] = (PositionMask >> 8) & 0xFF;
		HidOutput->LeftTrigger.Strengths.Compose[2] = ((FirstFootNib & 0x0F) << 4) | (SecondFootNib & 0x0F);
		HidOutput->LeftTrigger.Strengths.Compose[3] = static_cast<uint8>(Frequency);
	}

	if (Hand == EControllerHand::Right || Hand == EControllerHand::AnyHand)
	{
		HidOutput->RightTrigger.Mode = 0x23;
		HidOutput->RightTrigger.Strengths.Compose[0] = PositionMask & 0xFF;
		HidOutput->RightTrigger.Strengths.Compose[1] = (PositionMask >> 8) & 0xFF;
		HidOutput->RightTrigger.Strengths.Compose[2] = ((FirstFootNib & 0x0F) << 4) | (SecondFootNib & 0x0F);
		HidOutput->RightTrigger.Strengths.Compose[3] = static_cast<uint8>(Frequency);
	}

	SendOut();
}

void UDualSenseLibrary::SetMachine(int32 StartPosition, int32 EndPosition, int32 AmplitudeBegin,
                                   int32 AmplitudeEnd, float Frequency, float Period,
                                   const EControllerHand& Hand)
{
	FOutputContext* HidOutput = &HIDDeviceContexts.Output;
	const uint32_t Strengths = (((AmplitudeBegin & 0x07) << (3 * 0)) | ((AmplitudeEnd & 0x07) << (3 * 1)));

	if (Period < 0.0f || Period > 3.f) { Period = 3.f; }

	const uint16 PositionMask = (1 << StartPosition) | (1 << EndPosition);
	if (Hand == EControllerHand::Left || Hand == EControllerHand::AnyHand)
	{
		HidOutput->LeftTrigger.Mode = 0x27;
		HidOutput->LeftTrigger.Strengths.Compose[0] = PositionMask & 0xFF;
		HidOutput->LeftTrigger.Strengths.Compose[1] = 0x0;
		HidOutput->LeftTrigger.Strengths.Compose[2] = 0x0;
		HidOutput->LeftTrigger.Strengths.Compose[3] = 0x0;
		HidOutput->LeftTrigger.Strengths.Compose[4] = 0x0;
		HidOutput->LeftTrigger.Strengths.Compose[5] = 0x0;
		HidOutput->LeftTrigger.Strengths.Compose[6] = static_cast<uint8>(Frequency);
	}

	if (Hand == EControllerHand::Right || Hand == EControllerHand::AnyHand)
	{
		HidOutput->RightTrigger.Strengths.Compose[0] = PositionMask & 0xFF;
		HidOutput->RightTrigger.Strengths.Compose[1] = 0x0;
		HidOutput->RightTrigger.Strengths.Compose[2] = 0x0;
		HidOutput->RightTrigger.Strengths.Compose[3] = 0x0;
		HidOutput->RightTrigger.Strengths.Compose[4] = 0x0;
		HidOutput->RightTrigger.Strengths.Compose[5] = 0x0;
		HidOutput->RightTrigger.Strengths.Compose[6] = static_cast<uint8>(Frequency);
	}

	SendOut();
}

void UDualSenseLibrary::SetBow(int32 StartPosition, int32 EndPosition, int32 BegingStrength, int32 EndStrength,
                               const EControllerHand& Hand)
{
	HIDDeviceContexts.bOverrideTriggerBytes = false;
	FOutputContext* HidOutput = &HIDDeviceContexts.Output;


	if (StartPosition > 2 && StartPosition <= 4) { StartPosition = 4; }
	else if (StartPosition > 4 && StartPosition <= 6) { StartPosition = 8; }
	else if (StartPosition > 6) { StartPosition = 0; }
	else { StartPosition = 2; }

	if (BegingStrength > 2 && BegingStrength <= 6)
	{
		EndStrength = 15;
		BegingStrength = 2;
	}
	else if (BegingStrength > 6)
	{
		EndStrength = 15;
		BegingStrength = 3;
	}
	else
	{
		EndStrength = 0;
		BegingStrength = 10;
	}

	if (Hand == EControllerHand::Left || Hand == EControllerHand::AnyHand)
	{
		HidOutput->LeftTrigger.Mode = 0x22;
		HidOutput->LeftTrigger.Strengths.Compose[0] = (0x08 << 4) | (StartPosition & 0x0F);
		HidOutput->LeftTrigger.Strengths.Compose[1] = EndPosition == 8 ? 0x01 : 0x00;
		HidOutput->LeftTrigger.Strengths.Compose[2] = (BegingStrength & 0x0F) << 4 | (EndStrength & 0x0F);
	}

	if (Hand == EControllerHand::Right || Hand == EControllerHand::AnyHand)
	{
		HidOutput->RightTrigger.Mode = 0x22;
		HidOutput->RightTrigger.Strengths.Compose[0] = (0x08 << 4) | (StartPosition & 0x0F);
		HidOutput->RightTrigger.Strengths.Compose[1] = EndPosition == 8 ? 0x01 : 0x00;
		HidOutput->RightTrigger.Strengths.Compose[2] = (BegingStrength & 0x0F) << 4 | (EndStrength & 0x0F);
	}
	SendOut();
}

void UDualSenseLibrary::StopTrigger(const EControllerHand& Hand)
{
	FOutputContext* HidOutput = &HIDDeviceContexts.Output;
	if (Hand == EControllerHand::Left || Hand == EControllerHand::AnyHand) { HidOutput->LeftTrigger.Mode = 0x0; }

	if (Hand == EControllerHand::Right || Hand == EControllerHand::AnyHand) { HidOutput->RightTrigger.Mode = 0x0; }

	SendOut();
}

void UDualSenseLibrary::StopAll()
{
	FOutputContext* HidOutput = &HIDDeviceContexts.Output;
	if (
		HidOutput->Lightbar.A == 0 &&
		HidOutput->Lightbar.B == 0 &&
		HidOutput->Lightbar.R == 0
	) { HidOutput->Lightbar.B = 255; }

	HidOutput->PlayerLed.Led = static_cast<unsigned char>(ELedPlayerEnum::One);
	SendOut();
}

void UDualSenseLibrary::SetLightbar(FColor Color, float BrithnessTime, float ToggleTime)
{
	FOutputContext* HidOutput = &HIDDeviceContexts.Output;
	if ((HidOutput->Lightbar.R != Color.R) || (HidOutput->Lightbar.G != Color.G) || (HidOutput->Lightbar.B != Color.B))
	{
		HidOutput->Lightbar.R = Color.R;
		HidOutput->Lightbar.G = Color.G;
		HidOutput->Lightbar.B = Color.B;
		SendOut();
	}
}

void UDualSenseLibrary::SetPlayerLed(ELedPlayerEnum Led, ELedBrightnessEnum Brightness)
{
	FOutputContext* HidOutput = &HIDDeviceContexts.Output;
	if ((HidOutput->PlayerLed.Led != static_cast<unsigned char>(Led)) || (HidOutput->PlayerLed.Brightness !=
		static_cast
		<unsigned char>(Brightness)))
	{
		HidOutput->PlayerLed.Led = static_cast<unsigned char>(Led);
		HidOutput->PlayerLed.Brightness = static_cast<unsigned char>(Brightness);
		SendOut();
	}
}

void UDualSenseLibrary::SetMicrophoneLed(ELedMicEnum Led)
{
	FOutputContext* HidOutput = &HIDDeviceContexts.Output;
	if (HidOutput->MicLight.Mode != static_cast<unsigned char>(Led))
	{
		HidOutput->MicLight.Mode = static_cast<unsigned char>(Led);
		SendOut();
	}
}

void UDualSenseLibrary::EnableTouch(const bool bIsTouch) { bEnableTouch = bIsTouch; }

void UDualSenseLibrary::EnableMotionSensor(bool bIsMotionSensor)
{
	bEnableAccelerometerAndGyroscope = bIsMotionSensor;
}

bool UDualSenseLibrary::GetMotionSensorCalibrationStatus(float& OutProgress)
{
	if (!bIsCalibrating)
	{
		OutProgress = 1.0f;
		return false;
	}

	const double ElapsedTime = FPlatformTime::Seconds() - CalibrationStartTime;
	OutProgress = FMath::Clamp(ElapsedTime / CalibrationDuration, 0.0, 1.0);

	if (ElapsedTime >= CalibrationDuration)
	{
		if (CalibrationSampleCount > 0)
		{
			GyroBaseline.X = AccumulatedGyro.X / CalibrationSampleCount;
			GyroBaseline.Y = AccumulatedGyro.Y / CalibrationSampleCount;
			GyroBaseline.Z = AccumulatedGyro.Z / CalibrationSampleCount;

			AccelBaseline.X = AccumulatedAccel.X / CalibrationSampleCount;
			AccelBaseline.Y = AccumulatedAccel.Y / CalibrationSampleCount;
			AccelBaseline.Z = AccumulatedAccel.Z / CalibrationSampleCount;
		}
		bIsCalibrating = false;
		bHasMotionSensorBaseline = true;
		return false;
	}

	return true;
}

void UDualSenseLibrary::AudioHapticUpdate(TArray<int8> Data)
{
	FDeviceContext* Context = &HIDDeviceContexts;
	if (!Context || !Context->IsConnected) { return; }

	unsigned char* AudioData = &Context->BufferAudio[10];
	AudioData[0] = (AudioVibrationSequence++) & 0xFF;
	AudioData[1] = 0x92;
	AudioData[2] = 0x40;
	FMemory::Memcpy(&AudioData[3], Data.GetData(), 64);
	FPlayStationOutputComposer::SendAudioHapticAdvanced(Context);
}

void UDualSenseLibrary::StartMotionSensorCalibration(float Duration, float DeadZone)
{
	bIsCalibrating = true;
	CalibrationSampleCount = 0;

	GyroBaseline = FVector::ZeroVector;
	AccelBaseline = FVector::ZeroVector;
	AccumulatedGyro = FVector::ZeroVector;
	AccumulatedAccel = FVector::ZeroVector;

	SensorsDeadZone = FMath::Clamp(DeadZone, 0.0f, 1.f);
	CalibrationDuration = FMath::Clamp(Duration, 1.0f, 10.0f);
	CalibrationStartTime = FPlatformTime::Seconds();
}

void UDualSenseLibrary::SetHasPhoneConnected(const bool HasConnected) { HasPhoneConnected = HasConnected; }

void UDualSenseLibrary::SetLevelBattery(const float Level, bool FullyCharged, bool Charging)
{
	if (Level > 100.f)
	{
		LevelBattery = 100.f;
		return;
	}
	LevelBattery = Level;
}

void UDualSenseLibrary::CustomTrigger(const EControllerHand& Hand, const TArray<FString>& HexBytes)
{
	HIDDeviceContexts.bOverrideTriggerBytes = false;
	FOutputContext* OutBuffer = &HIDDeviceContexts.Output;

	uint8 Bytes[10] = {0};
	for (int32 i = 0; i < 10; ++i)
	{
		uint8 B = 0;
		if (!FValidateHelpers::ParseHexByte_Local(HexBytes[i], B))
		{
			UE_LOG(LogTemp, Warning, TEXT("CustomTrigger: invalid hex token at index %d: '%s'"), i, *HexBytes[i]);
			return;
		}
		Bytes[i] = B;
	}

	bool bIsValid;
	switch (Bytes[0])
	{
	case 0x01:
	case 0x02:
	case 0x21:
	case 0x22:
	case 0x23:
	case 0x25:
	case 0x26:
	case 0x27:
		bIsValid = true;
		break;
	default:
		bIsValid = false;
	}

	if (!bIsValid)
	{
		UE_LOG(LogTemp, Warning, TEXT("CustomTrigger: invalid hex token at index %d: '%s'"), 0, *HexBytes[0]);
		return;
	}

	if (Hand == EControllerHand::Left || Hand == EControllerHand::AnyHand)
	{
		OutBuffer->LeftTrigger.Mode = 0xFF;
		FMemory::Memset(OutBuffer->LeftTrigger.Strengths.Compose, 0, 10);
		FMemory::Memcpy(OutBuffer->LeftTrigger.Strengths.Compose, Bytes, 10);
	}

	if (Hand == EControllerHand::Right || Hand == EControllerHand::AnyHand)
	{
		OutBuffer->RightTrigger.Mode = 0xFF;
		FMemory::Memset(OutBuffer->RightTrigger.Strengths.Compose, 0, 10);
		FMemory::Memcpy(OutBuffer->RightTrigger.Strengths.Compose, Bytes, 10);
	}

	SendOut();
}
