
#include "m2uPluginPrivatePCH.h"
#include "Editor/UnrealEd/Private/FbxImporter.h"

Um2uFbxFactory::Um2uFbxFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}



/**
   This is called sometime before the Factory is used... It was once intended to 
   show a UI to ask for user input.
   This allows us to set stuff so the parent-class code won't show the UI, or even
   set values that we read from some input.
 */
bool Um2uFbxFactory::ConfigureProperties()
{
	// auto decide if StaticM or SkelM
	bDetectImportTypeOnImport = true;
	// do not show the UI
	bShowOption = false;
	
	// since we don't show the UI, not even the default-settings will be set,
	// so we do this here.
	UnFbx::FFbxImporter* FbxImporter = UnFbx::FFbxImporter::GetInstance();
	UnFbx::FBXImportOptions* ImportOptions = FbxImporter -> GetImportOptions();
	if( ImportOptions )
	{
		// General options
		ImportOptions -> bImportMaterials = false;
		ImportOptions -> bInvertNormalMap = false;
		ImportOptions -> bImportTextures = false;
		ImportOptions -> bImportLOD = false;
		ImportOptions -> bUsedAsFullName = true;
		ImportOptions -> bRemoveNameSpace = true;
		ImportOptions -> NormalImportMethod = FBXNIM_ImportNormals;
		// Static Mesh options
		ImportOptions -> bCombineToSingle = true;
		//ImportOptions -> bReplaceVertexColors = false;
		ImportOptions -> bRemoveDegenerates = true;
		ImportOptions -> bOneConvexHullPerUCX = true;
		//ImportOptions -> StaticMeshLODGroup = FName(TEXT("LevelArchitecture"));
		// Skeletal Mesh options 
		// TODO:... maybe
	}
	
	return true;
}
