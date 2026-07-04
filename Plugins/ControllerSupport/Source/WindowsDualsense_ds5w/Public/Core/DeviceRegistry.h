// Copyright (c) 2025 Rafael Valoto/Publisher. All rights reserved.
// Created for: WindowsDualsense_ds5w - Plugin to support DualSense controller on Windows.
// Planned Release Year: 2025

#pragma once

#include "CoreMinimal.h"
#include "HAL/PlatformProcess.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsApplication.h"
#endif

#include "Async/TaskGraphInterfaces.h"
#include "Interfaces/SonyGamepadInterface.h"

/**
 * A manager class that handles the creation, storage, and lifecycle management of device library
 * instances associated with Sony gamepad controllers. This class ensures proper initialization,
 * cleanup, and access control for the controller devices.
 */
class WINDOWSDUALSENSE_DS5W_API FDeviceRegistry : public TSharedFromThis<FDeviceRegistry>, public FNoncopyable
{
	/**
	 * Retrieves the static instance of the FDeviceRegistry class. This method
	 * ensures that only a single instance of the manager class is created and provides
	 * global access to it for managing device library instances of Sony gamepad controllers.
	 *
	 * @return A pointer to the singleton instance of UDeviceContainerManager. Returns nullptr
	 *         if the instance has not been initialized.
	 */
public:
	static TSharedPtr<FDeviceRegistry> Get();
	/**
	 * Destructor for the FDeviceRegistry class. Responsible for cleaning up resources associated with the device
	 * library instances. Ensures that all controller instances are cleaned up to prevent resource leakage.
	 */
	virtual ~FDeviceRegistry();
	/**
	 * Retrieves the total number of currently allocated device library instances.
	 * This method provides a count of the devices being managed by the container,
	 * useful for monitoring and ensuring proper resource allocation.
	 *
	 * @return The number of allocated device library instances.
	 */
	int32 GetAllocatedDevices();
	/**
	 * Retrieves the library instance associated with a specific Sony gamepad controller,
	 * identified by its unique controller ID. This method ensures that the appropriate
	 * library instance for the given controller is returned, enabling interaction with
	 * the corresponding input device.
	 *
	 * @param DeviceId The unique identifier of the Sony gamepad controller for which
	 *                     the library instance is to be retrieved.
	 * @return A pointer to the ISonyGamepadInterface instance corresponding to the specified
	 *         controller ID, or nullptr if no matching instance exists.
	 */
	ISonyGamepadInterface* GetLibraryInstance(const FInputDeviceId& DeviceId);
	/**
	 * Retrieves the map of allocated device library instances. This map associates unique integer
	 * keys with instances implementing the Sony gamepad interface, allowing access to the currently
	 * managed devices.
	 */
	TMap<FInputDeviceId, ISonyGamepadInterface*> GetAllocatedDevicesMap();
	/**
	 * Removes a library instance associated with the specified controller ID, disconnecting the
	 * corresponding input device if it is currently connected. Ensures proper removal and cleanup
	 * of the library instance from the internal container.
	 *
	 * @param GamepadId The unique identifier of the controller whose library instance is to be removed.
	 */
	void RemoveLibraryInstance(const FInputDeviceId& GamepadId);
	/**
	 * Creates an instance of a device library based on the provided device context. It initializes and
	 * manages the lifecycle of the Sony gamepad library for controllers like DualSense, DualSense Edge,
	 * or DualShock4. This method ensures proper mapping of device paths, unique IDs, and initialization
	 * of associated input devices.
	 *
	 * @param Context The device context containing the type of controller, its path,
	 *                and information required for initialization and identification.
	 */
	void CreateLibraryInstance(FDeviceContext& Context);
	/**
	 * Updates the device container manager at regular intervals by processing connected and
	 * disconnected devices. It handles device discovery, connection state updates, lifecycle
	 * management for device libraries, and ensures proper synchronization with previously
	 * known devices.
	 *
	 * @param DeltaTime The time in seconds since the last tick, used to accumulate time for
	 *                  periodic processing of the device lifecycle and connection state.
	 */
	void DetectedChangeConnections(float DeltaTime);
	
private:
	/**
	 * A floating-point variable that represents the change or difference in the accumulator value over time.
	 * Typically used to measure incremental adjustments or deltas in processing or calculations.
	 */
	float AccumulatorDelta = 0;
	/**
	 * A boolean variable indicating whether the device detection process is currently active.
	 * Used to track the ongoing state of device discovery and initialization within the device manager.
	 */
	bool bIsDeviceDetectionInProgress = false;
	/**
	 * A static instance of the FDeviceRegistry, serving as the singleton instance
	 * for managing device library instances of Sony gamepad controllers. This variable
	 * ensures global access and consistent management of the controller devices, while
	 * enforcing a single unique instance of the manager class.
	 */
	static TSharedPtr<FDeviceRegistry> Instance;
	/**
	 * A static map that holds associations between input device IDs and their corresponding Sony gamepad interface instances.
	 * This map is used to store and manage the lifecycle of gamepad interface objects, enabling efficient lookup and access
	 * to the respective implementations for handling Sony gamepad interactions.
	 */
	static TMap<FInputDeviceId, ISonyGamepadInterface*> LibraryInstances;
	/**
	 * A static map that associates device paths represented as strings with unique input device
	 * identifiers. This serves as a lookup mechanism to manage and track known device connections
	 * effectively within the system.
	 */
	static TMap<FString, FInputDeviceId> KnownDevicePaths;
	/**
	 * A static mapping that associates unique device identifiers with pairs containing input device IDs
	 * and platform user IDs. This data structure is used to track the history of devices interacting
	 * within the system, enabling efficient querying and management of device-user relationships.
	 */
	static TMap<FString, FInputDeviceId> HistoryDevices;
};
