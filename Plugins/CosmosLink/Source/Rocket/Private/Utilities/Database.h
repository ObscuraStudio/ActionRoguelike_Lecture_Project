// Copyright 2024 Leartes Studios. All Rights Reserved.

#pragma once

#include <sqlite3.h>

#include "CoreMinimal.h"

/**
 * Bridge is a class that provides an interface to interact with the SQLite database.
 */
class FDatabase {
	static sqlite3 *SQLiteDB; // The SQLite database.

	// Create whole database
	static bool CreateDatabase();

	// Create products table
	static bool CreateProductsTable();

	// Create settings table
	static bool CreateSettingsTable();

public:
	// Initializes the database
	static bool Initialize();

	// Executes a given SQL query and returns the result as a string.
	static FString Execute(const FString &SQLQuery);

	// Executes a SELECT SQL query and returns the result as a JSON string.
	static FString Select(const FString &SQLQuery);

	// Closes the SQLite database.
	static void Shutdown();
};
