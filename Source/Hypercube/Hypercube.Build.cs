// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Hypercube : ModuleRules
{
	public Hypercube(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "UMG", "NavigationSystem", "Niagara" });	
	}
}
