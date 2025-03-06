// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DEMO_MultiGame : ModuleRules
{
	public DEMO_MultiGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
	        "Core", "CoreUObject", "Engine", "InputCore", "NavigationSystem", "AIModule", "Niagara", "EnhancedInput"
	        , "OnlineSubsystem", "UMG"
        });
        
        PrivateIncludePaths.AddRange(
	        new string[] {
		        "DEMO_MultiGame" // 폴더 경로 추가
	        }
        );
    }
}
