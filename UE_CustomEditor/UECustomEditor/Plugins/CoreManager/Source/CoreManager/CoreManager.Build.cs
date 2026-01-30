// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CoreManager : ModuleRules
{
	public CoreManager(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				System.IO.Path.GetFullPath(Target.RelativeEnginePath) + "/Source/Editor/Blutility/Private" // Blutility 모듈을 참조하기 위한 엔진 상대경로 사용
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "Blutility",
                "EditorScriptingUtilities",
                "UMG",
				"Niagara",
                "UnrealEd",
                "AssetTools",
                "ContentBrowser",
                "LevelEditor",
				// ... add other public dependencies that you statically link with here ...
			}
            );
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
                "Slate",
                "SlateCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
