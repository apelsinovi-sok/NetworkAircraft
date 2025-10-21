// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NetworkAircraft : ModuleRules
{
	public NetworkAircraft(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG"
		});	
		
		PublicDependencyModuleNames.AddRange(new string[] {
			"PhysicsCore", "Chaos"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "AITestSuite", "AITestSuite" });

		PublicIncludePaths.AddRange(new string[] {
			"NetworkAircraft",
			"NetworkAircraft/Variant_Horror",
			"NetworkAircraft/Variant_Shooter",
			"NetworkAircraft/Variant_Shooter/AI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
