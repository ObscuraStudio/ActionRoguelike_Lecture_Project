// Copyright (c) 2025 Rafael Valoto/Publisher. All rights reserved.
// Created for: WindowsDualsense_ds5w - Plugin to support DualSense controller on Windows.
// Planned Release Year: 2025

#pragma once

template<typename T>
/**
 * @brief Represents device settings for configuration purposes.
 *
 * This struct is used to encapsulate settings required for configuring devices
 * or systems that rely on specific configurations. It provides a type-safe
 * mechanism to manage and apply settings efficiently.
 *
 * @tparam T The type of settings encapsulated by this structure.
 */
struct FSettings
{
	/**
	 * @brief Type alias representing configuration settings.
	 *
	 * This alias is used to handle device-related configuration settings within the system.
	 */
	T Settings;
};
