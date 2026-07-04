// Copyright 2024 Leartes Studios. All Rights Reserved.

#include "Rocket.h"
#include "Bridge.h"
#include "ContentBrowserDataMenuContexts.h"
#include "LevelEditor.h"
#include "SWebBrowser.h"
#include "Style.h"
#include "ToolMenus.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"

static const FName RocketTabName("Rocket");
static const FString CosmosURL = TEXT("https://cosmos.leartesstudios.com/");
static const FText RocketTabDisplay = FText::FromString("Rocket");
static const FText RocketTabDesc = FText::FromString("Launch Rocket");

const FName LevelEditorModuleName("LevelEditor");

#define LOCTEXT_NAMESPACE "Rocket"

TSharedPtr<FRocket> FRocket::Instance;
TSharedPtr<SWebBrowser> FRocket::WebBrowserWidget = nullptr;

void FRocket::Initialize() {
	if (!Instance.IsValid()) {
		Instance = MakeShareable(new FRocket);

		if (Instance.IsValid()) {
			RegisterMenus();
		}
	}
}

void FRocket::Shutdown() {
	if (Instance.IsValid()) {
		WebBrowserWidget.Reset();
		Instance.Reset();
	}

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(RocketTabName);
}

void FRocket::RegisterMenus() {
	FLevelEditorModule &LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(LevelEditorModuleName);
	const TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
	ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, nullptr, FToolBarExtensionDelegate::CreateStatic(&FRocket::FillToolbar));
	LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);

	// Adding Quick Menu Entry
	UToolMenu *AddMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.AddQuickMenu");
	FToolMenuSection &Section = AddMenu->FindOrAddSection("Content");
	Section.AddMenuEntry("OpenRocketTab", RocketTabDisplay, RocketTabDesc, FSlateIcon(FStyle::GetToolStyleName(), "Rocket.RocketLogo"), FUIAction(FExecuteAction::CreateStatic(&FRocket::OpenBrowserRequested), FCanExecuteAction())).InsertPosition = FToolMenuInsert("ImportContent", EToolMenuInsertType::After);

	UToolMenu *WindowMenu = UToolMenus::Get()->ExtendMenu("MainFrame.MainMenu.Window");
	FToolMenuSection *ContentSectionPtr = WindowMenu->FindSection("GetContent");
	if (!ContentSectionPtr) {
		ContentSectionPtr = &WindowMenu->AddSection("GetContent", NSLOCTEXT("MainAppMenu", "GetContentHeader", "Get Content"));
	}
	ContentSectionPtr->AddMenuEntry("OpenRocketTab", RocketTabDisplay, RocketTabDesc, FSlateIcon(FStyle::GetToolStyleName(), "Rocket.RocketLogo"), FUIAction(FExecuteAction::CreateStatic(&FRocket::OpenBrowserRequested), FCanExecuteAction()));

	UToolMenu *ContextMenu = UToolMenus::Get()->ExtendMenu("ContentBrowser.AddNewContextMenu");
	FToolMenuSection &ContextMenuSection = ContextMenu->FindOrAddSection("ContentBrowserGetContent");

	FToolMenuEntry& ToolMenuEntry = UToolMenus::Get()->ExtendMenu("ContentBrowser.Toolbar")->FindOrAddSection("New").AddEntry(
	FToolMenuEntry::InitToolBarButton(
		"OpenRocketWindow",
		FToolUIActionChoice(
			FToolUIAction(FToolMenuExecuteAction::CreateLambda([](const FToolMenuContext& InContext)
			{
				OpenBrowserRequested();
			}))),
		RocketTabDisplay,
		RocketTabDesc,
		FSlateIcon(FStyle::GetToolStyleName(), "Rocket.RocketLogo"),
		EUserInterfaceActionType::Button
	));
	ToolMenuEntry.StyleNameOverride = FName("CalloutToolbar");

	TWeakPtr<FRocket> WeakPtr = Instance.ToSharedRef();
	ContextMenuSection.AddDynamicEntry("GetRocket", FNewToolMenuSectionDelegate::CreateLambda([WeakPtr](FToolMenuSection &InSection) {
		const UContentBrowserDataMenuContext_AddNewMenu *AddNewMenuContext = InSection.FindContext<UContentBrowserDataMenuContext_AddNewMenu>();
		if (AddNewMenuContext && AddNewMenuContext->bCanBeModified && AddNewMenuContext->bContainsValidPackagePath && WeakPtr.IsValid()) {
			InSection.AddMenuEntry("GetRocket", RocketTabDisplay, RocketTabDesc, FSlateIcon(FStyle::GetToolStyleName(), "Rocket.RocketLogo"), FUIAction(FExecuteAction::CreateStatic(&FRocket::OpenBrowserRequested), FCanExecuteAction()));
		}
	}));

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(RocketTabName, FOnSpawnTab::CreateRaw(Instance.Get(), &FRocket::OnGenerateRocketTab)).SetDisplayName(RocketTabDisplay).SetTooltipText(RocketTabDesc).SetAutoGenerateMenuEntry(false);
}

void FRocket::FillToolbar(FToolBarBuilder &ToolbarBuilder) {
	ToolbarBuilder.BeginSection(TEXT("Rocket"));
	{
		ToolbarBuilder.AddToolBarButton(FUIAction(FExecuteAction::CreateStatic(&FRocket::OpenBrowserRequested), FCanExecuteAction()), FName(TEXT("Rocket")), RocketTabDisplay, RocketTabDesc, FSlateIcon(FStyle::GetToolStyleName(), "RocketLogo"), EUserInterfaceActionType::Button, FName(TEXT("Rocket")));
	}
	ToolbarBuilder.EndSection();
}

void FRocket::OpenBrowserRequested() {
	FGlobalTabmanager::Get()->TryInvokeTab(RocketTabName);
}

void FRocket::CreateBrowserWidget() {
	if (!WebBrowserWidget.IsValid()) {
		WebBrowserWidget = SNew(SWebBrowser).InitialURL(CosmosURL).ShowControls(false).Visibility(EVisibility::Hidden);
	}
}

TSharedRef<SDockTab> FRocket::OnGenerateRocketTab(const FSpawnTabArgs &SpawnTabArgs) {
	CreateBrowserWidget();

	if (!BrowserDock.IsValid()) {
		SAssignNew(BrowserDock, SDockTab).OnTabClosed_Lambda([this](TSharedRef<SDockTab> InParentTab) {
			BrowserDock.Reset();
		}).TabRole(NomadTab)[WebBrowserWidget.ToSharedRef()];

		if (!UBridge::Bridge) {
			UBridge::Bridge = NewObject<UBridge>();
			WebBrowserWidget->BindUObject(TEXT("Bridge"), UBridge::Bridge);
		}
	} else {
		WebBrowserWidget->SetVisibility(EVisibility::Visible);
	}

	return BrowserDock.ToSharedRef();
}

#undef LOCTEXT_NAMESPACE
