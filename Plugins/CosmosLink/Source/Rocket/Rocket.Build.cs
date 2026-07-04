// Copyright 2024 Leartes Studios. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class Rocket : ModuleRules
{
	public Rocket(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private/Data"));
		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private/Download"));
		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private/Utilities"));

		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "../ThirdParty/sqlite3"));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "../ThirdParty/miniz"));

		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"Core",
				"HTTP"
			});

		if (Target.Version.MajorVersion == 5 && Target.Version.MinorVersion <= 0)
		{
			PublicDependencyModuleNames.Add("EditorStyle");
		}
		
		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"Projects",
				"InputCore",
				"UnrealEd",
				"ToolMenus",
				"RenderCore",
				"RHI",
				"UMG",
				"ImageWrapper",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"WebBrowser",
				"AssetRegistry",
				"ContentBrowserData",
				"DesktopPlatform",
				"Json",
				"JsonUtilities",
				"Niagara",
				"NiagaraEditor",
				"ContentBrowser",
				"MaterialUtilities",
				"AudioEditor",
				"LevelEditor",
				"GameProjectGeneration", 
				"PluginBrowser", 
				"ToolWidgets", 
				"SettingsEditor",
			}
		);
	}
}