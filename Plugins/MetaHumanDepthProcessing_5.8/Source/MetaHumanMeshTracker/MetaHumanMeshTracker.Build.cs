// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildBase;
using UnrealBuildTool;
using System.IO;

public class MetaHumanMeshTracker : ModuleRules
{
	protected bool BuildForDevelopment
	{
		get
		{
			// Check if source is available
			string SourceFilesPath = Path.Combine(ModuleDirectory, "Private");
			return Directory.Exists(SourceFilesPath) &&
				   Directory.GetFiles(SourceFilesPath).Length > 0;
		}
	}

	public MetaHumanMeshTracker(ReadOnlyTargetRules Target) : base(Target)
	{
		bUsePrecompiled = !BuildForDevelopment;
		bRequiresPlatformSDK = true;
		IWYUSupport = IWYUSupport.None;
		CppCompileWarningSettings.UndefinedIdentifierWarningLevel = WarningLevel.Off;
		CppCompileWarningSettings.ShadowVariableWarningLevel = WarningLevel.Warning;
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bUseUnity = false;
		PrivatePCHHeaderFile = "Private/MetaHumanMeshTrackerPrivatePCH.h";

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"MetaHumanCoreTechLib",
			"MetaHumanCoreTech",
			"CaptureDataCore",
			"MeshTrackerInterface",
			"MetaHumanMeshTrackerCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"Vulkan",
			"Eigen",
			"RigLogicLib",
			"RigLogicModule",
			"DNACalibLib",
			"simde",
		});


		PrivateDependencyModuleNames.AddRange(new string[] {
		});

		if (Target.Type == TargetType.Editor)
		{
			PrivateDependencyModuleNames.Add("UnrealEd");
		}

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true

		PrivateIncludePaths.AddRange(new string[]
		{
			Path.Combine(ModuleDirectory, "../ThirdParty/dlib/Include"),
			Path.Combine(ModuleDirectory, "Private/carbon/include"),
			Path.Combine(ModuleDirectory, "Private/nls/include"),
			Path.Combine(ModuleDirectory, "Private/nrr/include"),
			Path.Combine(ModuleDirectory, "Private/reconstruction/include"),
			Path.Combine(ModuleDirectory, "Private/tracking/include"),
			Path.Combine(ModuleDirectory, "Private/vulkantools/include"),
			Path.Combine(ModuleDirectory, "Private/ThirdParty/optflow/include"),
			Path.Combine(ModuleDirectory, "Private/predictivesolver/include"),
			Path.Combine(ModuleDirectory, "Private/api"),
			Path.Combine(ModuleDirectory, "Private/rlibv/include"),
			Path.Combine(ModuleDirectory, "Private/rlibv/include/ThirdParty/dlib"),
			Path.Combine(ModuleDirectory, "Private/rig/include"),
			Path.Combine(ModuleDirectory, "Private/resourceloader/include")
		});

		PrivateDefinitions.Add("TITAN_DYNAMIC_API");
		PrivateDefinitions.Add("LOG_INTEGRATION");
		PrivateDefinitions.Add("WITH_VMA");
		PrivateDefinitions.Add("TITAN_NAMESPACE=mhdp::epic::nls");
		PrivateDefinitions.Add("TITAN_API_NAMESPACE=mhdp::titan::api");

		PrivateDefinitions.Add("CARBON_ENABLE_SSE=1");
		PrivateDefinitions.Add("CARBON_ENABLE_AVX=0"); // CARBON_ENABLE_AVX does not work with Windows clang build

		// This module uses exceptions in the core tech libs, so they must be enabled here
		bEnableExceptions = true;

		// AutoRTFM cannot be used with exceptions.
		bDisableAutoRTFMInstrumentation = true;
		
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			if (Target.Architecture == UnrealArch.Arm64)
			{
				PublicAdditionalLibraries.Add("$(ModuleDir)/../ThirdParty/dlib/Lib/WinArm64/Release/dlib19.23.0_release_64bit_msvc1929.lib");
			}
			else
			{
				PublicAdditionalLibraries.Add("$(ModuleDir)/../ThirdParty/dlib/Lib/Win64/Release/dlib19.23.0_release_64bit_msvc1929.lib");
			}
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			PublicAdditionalLibraries.Add("$(ModuleDir)/../ThirdParty/dlib/Lib/Mac/Release/libdlib.a");

			// MoltenVK: Vulkan loader + MoltenVK ICD + manifest for Vulkan-via-Metal on Mac
			string VulkanMacBinDir = Path.Combine(Unreal.EngineDirectory.ToString(), "Binaries/ThirdParty/Vulkan/Mac");
			RuntimeDependencies.Add(Path.Combine(VulkanMacBinDir, "libvulkan.dylib"), StagedFileType.NonUFS);
			RuntimeDependencies.Add(Path.Combine(VulkanMacBinDir, "libMoltenVK.dylib"), StagedFileType.NonUFS);
			RuntimeDependencies.Add(Path.Combine(VulkanMacBinDir, "MoltenVK_icd.json"), StagedFileType.NonUFS);

			// Frameworks required by MoltenVK at runtime
			PublicFrameworks.AddRange(new string[] { "Metal", "IOSurface", "QuartzCore" });
		}
		else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			PublicAdditionalLibraries.Add("$(ModuleDir)/../ThirdParty/dlib/Lib/Linux/Release/libdlib.a");
		}
		// Need to disable unity builds for this module to avoid clang compilation issues with the TEXT macro
		//bUseUnity = false;

	}
}
