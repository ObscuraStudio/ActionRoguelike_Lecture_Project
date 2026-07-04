// Copyright (c) 2025 Rafael Valoto/Publisher. All rights reserved.
// Created for: WindowsDualsense_ds5w - Plugin to support DualSense controller on Windows.
// Planned Release Year: 2025

#pragma once

#if PLATFORM_WINDOWS
#else
#include "Core/Interfaces/PlatformHardwareInfoInterface.h"
class FCommonsDeviceInfo : public IPlatformHardwareInfoInterface 
{
	/**
	 * Virtual destructor for the FCommonsDeviceInfo class.
	 *
	 * This destructor ensures proper cleanup of resources associated with
	 * the FCommonsDeviceInfo class when the object is destroyed. It overrides
	 * the destructor of the IPlatformHardwareInfoInterface base class.
	 *
	 * The destructor is declared as virtual to allow correct destruction
	 * of derived class objects through base class pointers.
	 */
public:
	virtual ~FCommonsDeviceInfo() override {};
	/**
	 * Processes audio haptic feedback using the given device context.
	 *
	 * This method is responsible for handling and generating audio-based
	 * haptic feedback through the provided device context. It leverages
	 * the context to access or manage the required resources for delivering
	 * the haptic feedback.
	 *
	 * @param Context A pointer to the FDeviceContext object, which provides
	 *                the necessary interface and data for processing audio
	 *                haptic feedback. Must be a valid, properly initialized
	 *                pointer.
	 */
	virtual void ProcessAudioHapitc(FDeviceContext* Context) override;
	/**
	 * Reads device information using the provided device context.
	 *
	 * This method retrieves and processes the relevant information about
	 * the device through the given device context. It is a member function
	 * of the FCommonsDeviceInfo class and is intended to handle device-specific
	 * data retrieval operations.
	 *
	 * @param Context A pointer to the FDeviceContext object, which provides
	 *        the necessary context and interface for accessing device-related
	 *        information. It must be valid and properly initialized.
	 */
	virtual void Read(FDeviceContext* Context) override;
	/**
	 * Writes device-specific information to the provided device context.
	 *
	 * This method is responsible for processing and transferring relevant
	 * device information to the supplied context. It is expected to interact
	 * with the context to store or manipulate data as needed for device operations.
	 *
	 * @param Context Pointer to the FDeviceContext object that serves as the
	 *                target for the device information. Must be a valid,
	 *                non-null pointer.
	 */
	virtual void Write(FDeviceContext* Context) override;
	/**
	 * Detects and populates a list of device contexts.
	 *
	 * This method scans for available devices and collects their information,
	 * storing it in the provided array of FDeviceContext objects.
	 *
	 * @param Devices A reference to an array of FDeviceContext objects
	 *                that will be populated with the detected device information.
	 */
	virtual void Detect(TArray<FDeviceContext>& Devices) override;
	/**
	 * Creates a handle for the FCommonsDeviceInfo class using the provided device context.
	 *
	 * This method initializes and configures a handle specific to the device context
	 * supplied as an argument. It is expected to ensure that the device context is
	 * used properly in the creation of the handle.
	 *
	 * @param Context A pointer to the FDeviceContext instance used for handle creation.
	 * @return A boolean value indicating whether the handle was successfully created.
	 */
	virtual bool CreateHandle(FDeviceContext* Context) override;
	/**
	 * Invalidates the device handle associated with the specified device context.
	 *
	 * This method clears the association of the handle linked with the provided
	 * FDeviceContext, ensuring that it is no longer valid. It is typically used
	 * during device cleanup or reinitialization processes.
	 *
	 * @param Context A pointer to the FDeviceContext instance whose handle is to
	 *                be invalidated. This parameter must not be null.
	 */
	virtual void InvalidateHandle(FDeviceContext* Context) override;
};
#endif
