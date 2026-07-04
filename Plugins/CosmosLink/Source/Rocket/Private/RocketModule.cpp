// Copyright 2024 Leartes Studios. All Rights Reserved.

#include "RocketModule.h"
#include "Commands.h"
#include "Database.h"
#include "FileDownloadHandler.h"
#include "Rocket.h"
#include "Style.h"
#include "Utilities/SourceManager.h"

#define LOCTEXT_NAMESPACE "Rocket"

class FRocketModule final : public IRocketModule {
public:
	virtual void StartupModule() override {
		if (GIsEditor && !IsRunningCommandlet()) {
			FSourceManager::InitializeSourceDirectory();

			FCommands::RegisterCommands();

			FDatabase::Initialize();

			FStyle::Initialize();

			FRocket::Initialize();

			FCoreDelegates::OnPreExit.AddStatic(&FRocketModule::OnEngineShutdown);
		}
	}

	virtual void ShutdownModule() override {
		FCoreDelegates::OnPreExit.RemoveAll(this);

		FDatabase::Shutdown();

		FCommands::UnregisterCommands();

		FRocket::Shutdown();

		FStyle::Shutdown();

		UFileDownloadHandler::Cleanup();
	}

	static void OnEngineShutdown() {

		FDatabase::Shutdown();

		FRocket::Shutdown();

		FStyle::Shutdown();

		UFileDownloadHandler::Cleanup();
	}
};

IMPLEMENT_MODULE(FRocketModule, Rocket);

#undef LOCTEXT_NAMESPACE

DEFINE_LOG_CATEGORY(LogRocket);
