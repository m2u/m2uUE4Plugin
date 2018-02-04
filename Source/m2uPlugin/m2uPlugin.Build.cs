// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class m2uPlugin : ModuleRules
	{
		public m2uPlugin(ReadOnlyTargetRules Target) : base(Target)
		{			

			PublicIncludePaths.AddRange(
				new string[] {
					"Developer/AssetTools/Public",
					"Editor/UnrealEd/Public",
					"Editor/UnrealEd/Classes",					  
					// ... add public include paths required here ...
				}
				);

			PrivateIncludePaths.AddRange(
				new string[] {
					"Developer/m2uPlugin/Private",					
					"Editor/UnrealEd/Public",
					"Editor/UnrealEd/Classes",
						"Editor/UnrealEd/Private",
						"Editor/UnrealEd/Private/Fbx",
					// ... add other private include paths required here ...
				}
				);

			AddEngineThirdPartyPrivateStaticDependencies(Target, "FBX");

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
				"Core",
				"CoreUObject",
                "InputCore",
                "EditorStyle",
				"Engine",
				"UnrealEd",
				"Sockets",
				"Networking",
				"Slate",
				"SlateCore",
				"Projects",
					// ... add other public dependencies that you statically link with here ...
				}
				);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"UnrealEd",
					"Json",
					// ... add private dependencies that you statically link with here ...
				}
				);

			DynamicallyLoadedModuleNames.AddRange(
				new string[]
				{
					"AssetTools",
					// ... add any modules that your module loads dynamically here ...
				}
				);
		}
	}
}
