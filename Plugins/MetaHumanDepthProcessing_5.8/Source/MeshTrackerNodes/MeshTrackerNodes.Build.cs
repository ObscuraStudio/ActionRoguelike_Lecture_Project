// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class MeshTrackerNodes : ModuleRules
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
	public MeshTrackerNodes(ReadOnlyTargetRules Target) : base(Target)
	{
		bUsePrecompiled = !BuildForDevelopment;
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
			}
		);	
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"MeshTrackerInterface",
				"MetaHumanIdentity",
				"MetaHumanMeshTracker",
			}
		);
	}
}