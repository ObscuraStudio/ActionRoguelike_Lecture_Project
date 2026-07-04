// Copyright (c) 2025 Rafael Valoto/Publisher. All rights reserved.
// Created for: WindowsDualsense_ds5w - Plugin to support DualSense controller on Windows.
// Planned Release Year: 2025

#include "../../Public/Subsystems/AudioHapticsListener.h"
#include "Core/DeviceRegistry.h"
#include "Core/Interfaces/SonyGamepadTriggerInterface.h"
#include "Core/Structs/DualSenseFeatureReport.h"

FAudioHapticsListener::FAudioHapticsListener(FInputDeviceId InDeviceId, USoundSubmix* InSubmix) : Submix(InSubmix), DeviceId(InDeviceId)
{
	ResampledAudioBuffer.SetNumUninitialized(64);
}

void FAudioHapticsListener::OnNewSubmixBuffer(const USoundSubmix* OwningSubmix, float* AudioData, int32 NumSamples,
                                              int32 NumChannels, const int32 SampleRate, double AudioClock)
{
    if (!ResamplerImpl.IsValid())
    {
        const float Ratio = 3000.0f / SampleRate;
        ResamplerImpl = MakeUnique<Audio::FResampler>();
        ResamplerImpl->Init(
            Audio::EResamplingMethod::BestSinc,
            Ratio,
            NumChannels
        );
    }
    const int32 NumInputFrames = NumSamples / NumChannels; // (2048 samples / 2 channels = 1024 frames)

    // (1024 frames * (3000/48000)) = 64 frames.
    const int32 ExpectedOutputFrames = FMath::CeilToInt(static_cast<float>(NumInputFrames) * (3000.0f / SampleRate));
    ResampledAudioBuffer.SetNumUninitialized((ExpectedOutputFrames + 32) * NumChannels);

    int32 OutputFramesWritten = 0;
    ResamplerImpl->ProcessAudio(
        AudioData,
        NumInputFrames,
        false,
        ResampledAudioBuffer.GetData(),
        ResampledAudioBuffer.Num() / NumChannels,
        OutputFramesWritten
    );

    if (OutputFramesWritten != 64)
    {
        UE_LOG(LogTemp, Warning, TEXT("OutputFramesWritten not 64 bytes! (%d)"), OutputFramesWritten);
        return;
    }

	const float alpha = 0.2f;
	const float one_minus_alpha = 0.5f - alpha;
	
	float* Data = ResampledAudioBuffer.GetData();
	const int32 NumFrames = OutputFramesWritten; // Serão 64 frames

	for (int32 i = 0; i < NumFrames; ++i)
	{
		const int32 DataIndex = i * NumChannels; // (i * 2)
  
		const float InLeft  = Data[DataIndex];
		const float InRight = Data[DataIndex + 1];
		
		// y_lp[n] = (1 - alpha) * x[n] + alpha * y_lp[n-1]
		LowPassState_Left  = one_minus_alpha * InLeft  + alpha * LowPassState_Left;
		LowPassState_Right = one_minus_alpha * InRight + alpha * LowPassState_Right;
		
		// y_hp[n] = x[n] - y_lp[n]
		const float OutLeft  = InLeft  - LowPassState_Left;
		const float OutRight = InRight - LowPassState_Right;
		
		Data[DataIndex]     = OutLeft;
		Data[DataIndex + 1] = OutRight;
	}

    const float* ResampledData = ResampledAudioBuffer.GetData();
    TArray<int8> Packet1, Packet2;
    Packet1.SetNumUninitialized(64);
    Packet2.SetNumUninitialized(64);

    for (int32 i = 0; i < 32; ++i)
    {
        const int32 DataIndex = i * 2;
        
        const float LeftSample  = ResampledData[DataIndex]; // L
        const float RightSample = ResampledData[DataIndex + 1]; // R

        const int8 LeftSampleInt8  = static_cast<int8>(FMath::Clamp(FMath::RoundToInt(LeftSample * 127.0f), -128, 127));
        const int8 RightSampleInt8 = static_cast<int8>(FMath::Clamp(FMath::RoundToInt(RightSample * 127.0f), -128, 127));

        Packet1[DataIndex]     = LeftSampleInt8;  // L
        Packet1[DataIndex + 1] = RightSampleInt8; // R
    }
    
    // (Frames 32-63)
    for (int32 i = 0; i < 32; ++i)
    {
        // Freame 32 (i + 32) * 2
        const int32 DataIndex = (i + 32) * 2; // ResampledData (64, 66, 68...)
        
        const float LeftSample  = ResampledData[DataIndex];
        const float RightSample = ResampledData[DataIndex + 1];

        const int8 LeftSampleInt8  = static_cast<int8>(FMath::Clamp(FMath::RoundToInt(LeftSample * 127.0f), -128, 127));
        const int8 RightSampleInt8 = static_cast<int8>(FMath::Clamp(FMath::RoundToInt(RightSample * 127.0f), -128, 127));

        // Index *Packet 2* (Packet2)
        const int32 PacketIndex = i * 2; // (0, 2, 4...)
        Packet2[PacketIndex]     = LeftSampleInt8;  // L
        Packet2[PacketIndex + 1] = RightSampleInt8; // R
    }

    AudioPacketQueue.Enqueue(Packet1);
    AudioPacketQueue.Enqueue(Packet2);
}

void FAudioHapticsListener::ConsumeHapticsQueue()
{
    ISonyGamepadTriggerInterface* DualSenseInterface = Cast<ISonyGamepadTriggerInterface>(
        FDeviceRegistry::Get()->GetLibraryInstance(DeviceId));
    if (DualSenseInterface)
	{
		TArray<int8> PacketToProcess;
	    while (AudioPacketQueue.Dequeue(PacketToProcess))
	    {
	        DualSenseInterface->AudioHapticUpdate(PacketToProcess);
	    }
    	return;
	}
	AudioPacketQueue.Empty();
}
