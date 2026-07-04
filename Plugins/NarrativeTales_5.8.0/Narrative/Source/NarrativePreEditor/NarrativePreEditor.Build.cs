// Copyright Narrative Tools 2025. 

using UnrealBuildTool;

public class NarrativePreEditor : ModuleRules
{
	public NarrativePreEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "UnrealEd",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "InputCore",
                "GraphEditor",
                "KismetWidgets",
                "PropertyEditor", "Narrative"
            }
        );
    }
}
