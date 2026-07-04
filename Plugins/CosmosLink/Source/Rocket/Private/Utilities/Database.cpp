// Copyright 2024 Leartes Studios. All Rights Reserved.

#include "Database.h"
#include "PathHelpers.h"
#include "RocketModule.h"
#include "Logging/LogMacros.h"
#include "Misc/Paths.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

sqlite3 *FDatabase::SQLiteDB = nullptr;

bool FDatabase::Initialize() {
	if (!CreateDatabase()) {
		return false;
	}

	if (!CreateProductsTable()) {
		return false;
	}

	if (!CreateSettingsTable()) {
		return false;
	}

	return true;
}

bool FDatabase::CreateDatabase() {
	const FString DBFilePath = FPaths::Combine(FPathHelpers::GetRocketDepotPath(), TEXT("db.sqlite"));

	int Result = sqlite3_open(TCHAR_TO_UTF8(*DBFilePath), &SQLiteDB);
	if (Result != SQLITE_OK) {
		const FString ErrorString = UTF8_TO_TCHAR(sqlite3_errmsg(SQLiteDB));
		UE_LOG(LogRocket, Error, TEXT("Failed to open/create database: %s"), *ErrorString);
		return false;
	}

	Result = sqlite3_exec(SQLiteDB, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
	Result &= sqlite3_exec(SQLiteDB, "PRAGMA synchronous=NORMAL;", nullptr, nullptr, nullptr);
	Result &= sqlite3_exec(SQLiteDB, "PRAGMA cache_size=5000;", nullptr, nullptr, nullptr);

	if (Result != SQLITE_OK) {
		UE_LOG(LogRocket, Error, TEXT("Failed to configure database settings"));
		sqlite3_close(SQLiteDB);
		SQLiteDB = nullptr;
		return false;
	}
	return true;
}

bool FDatabase::CreateProductsTable() {
	const char *CreateTableQuery = "CREATE TABLE IF NOT EXISTS products (" "id TEXT PRIMARY KEY, " "title TEXT, " "slug TEXT, " "type TEXT, " "cover_image TEXT, " "options TEXT);";

	char *ErrorMessage = nullptr;
	const int Result = sqlite3_exec(SQLiteDB, CreateTableQuery, nullptr, nullptr, &ErrorMessage);
	if (Result != SQLITE_OK) {
		UE_LOG(LogRocket, Error, TEXT("Failed to create products table: %s (SQLite error code: %d)"), UTF8_TO_TCHAR(ErrorMessage), Result);
		sqlite3_free(ErrorMessage);
		sqlite3_close(SQLiteDB);
		SQLiteDB = nullptr;
		return false;
	}

	//UE_LOG(LogRocket, Log, TEXT("Products table created successfully"));
	return true;
}

bool FDatabase::CreateSettingsTable() {
	const char *CreateSettingsTableQuery = "CREATE TABLE IF NOT EXISTS settings (" "key TEXT PRIMARY KEY, " "value TEXT);";

	char *ErrorMessage = nullptr;
	int Result = sqlite3_exec(SQLiteDB, CreateSettingsTableQuery, nullptr, nullptr, &ErrorMessage);
	if (Result != SQLITE_OK) {
		FString ErrorString = UTF8_TO_TCHAR(ErrorMessage);
		UE_LOG(LogRocket, Error, TEXT("Failed to create settings table: %s (SQLite error code: %d)"), *ErrorString, Result);
		sqlite3_free(ErrorMessage);
		sqlite3_close(SQLiteDB);
		SQLiteDB = nullptr;
		return false;
	}

	FString HomeDirectory = FPathHelpers::GetHomeDirectory();
	const FString ExtractionPath = FPaths::Combine(HomeDirectory, TEXT("Documents"), TEXT("Rocket")).Replace(TEXT("\\"), TEXT("/"));

	const FString InsertDefaultSettingsQuery = FString::Printf(TEXT("INSERT OR IGNORE INTO settings (key, value) VALUES " "('dropQuality', 'low'), " "('extractionPath', '%s');"), *ExtractionPath);

	Result = sqlite3_exec(SQLiteDB, TCHAR_TO_UTF8(*InsertDefaultSettingsQuery), nullptr, nullptr, &ErrorMessage);
	if (Result != SQLITE_OK) {
		FString ErrorString = UTF8_TO_TCHAR(ErrorMessage);
		UE_LOG(LogRocket, Error, TEXT("Failed to insert default settings: %s (SQLite error code: %d)"), *ErrorString, Result);
		sqlite3_free(ErrorMessage);
		sqlite3_close(SQLiteDB);
		SQLiteDB = nullptr;
		return false;
	}

	//UE_LOG(LogRocket, Log, TEXT("Settings table created and default settings inserted successfully"));
	return true;
}

FString FDatabase::Execute(const FString &SQLQuery) {
	if (!SQLiteDB) {
		UE_LOG(LogRocket, Error, TEXT("Database connection is not initialized. Cannot execute query."));
		return TEXT("Database connection is not initialized.");
	}

	char *ErrMsg = nullptr;
	int Result = sqlite3_exec(SQLiteDB, TCHAR_TO_UTF8(*SQLQuery), nullptr, nullptr, &ErrMsg);

	if (Result != SQLITE_OK) {
		FString ErrorString = UTF8_TO_TCHAR(ErrMsg);
		UE_LOG(LogRocket, Error, TEXT("Failed to execute SQL query: %s"), *ErrorString);
		sqlite3_free(ErrMsg);
		return ErrorString;
	}
	return TEXT("SQL query executed successfully.");
}

FString FDatabase::Select(const FString &SQLQuery) {
	if (!SQLiteDB) {
		UE_LOG(LogRocket, Error, TEXT("Database connection is not initialized. Cannot select query."));
		return TEXT("Database connection is not initialized.");
	}

	sqlite3_stmt *Statement;
	const int Result = sqlite3_prepare_v2(SQLiteDB, TCHAR_TO_UTF8(*SQLQuery), -1, &Statement, nullptr);

	if (Result != SQLITE_OK) {
		FString ErrorString = UTF8_TO_TCHAR(sqlite3_errmsg(SQLiteDB));
		UE_LOG(LogRocket, Error, TEXT("Failed to prepare SQL statement: %s"), *ErrorString);
		return ErrorString;
	}

	FString ResultString;
	TArray<TSharedPtr<FJsonObject>> Results;

	while (sqlite3_step(Statement) == SQLITE_ROW) {
		TSharedPtr<FJsonObject> Row = MakeShareable(new FJsonObject);

		int ColumnCount = sqlite3_column_count(Statement);
		for (int i = 0; i < ColumnCount; ++i) {
			FString ColName = UTF8_TO_TCHAR(sqlite3_column_name(Statement, i));
			switch (sqlite3_column_type(Statement, i)) {
			case SQLITE_INTEGER:
				Row->SetNumberField(ColName, sqlite3_column_int(Statement, i));
				break;
			case SQLITE_FLOAT:
				Row->SetNumberField(ColName, sqlite3_column_double(Statement, i));
				break;
			case SQLITE_TEXT:
				Row->SetStringField(ColName, UTF8_TO_TCHAR(reinterpret_cast<const char *>(sqlite3_column_text(Statement, i))));
				break;
			default:
				Row->SetStringField(ColName, TEXT("Unsupported Type"));
				break;
			}
		}

		Results.Add(Row);
	}

	sqlite3_finalize(Statement);

	TArray<TSharedPtr<FJsonValue>> JsonValues;
	for (const TSharedPtr<FJsonObject> &JsonObject : Results) {
		JsonValues.Add(MakeShareable(new FJsonValueObject(JsonObject)));
	}

	TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&ResultString);
	FJsonSerializer::Serialize(JsonValues, Writer);

	return ResultString;
}

void FDatabase::Shutdown() {
	if (SQLiteDB) {
		sqlite3_close(SQLiteDB);
		SQLiteDB = nullptr;
	}
}
