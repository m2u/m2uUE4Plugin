
#pragma once
//#include "Source/Editor/UnrealEd/Classes/FbxFactory.h"
#include "UnrealEd.h"
#include "m2uFbxFactory.generated.h"


/**
   This is a "silent" FBX-factory which won't generate a popup dialog to ask for 
   options.
   The default FBX-factory can't be told to not do that, because everything is
   protected, unaccesible and hard coded to be intertwined with the UI.
   The import settings data is stored in the UI and can only be changed from within
   the class.
   The only way around showing the UI would be to set GIsAutomationTesting true.
   That may have side effects I don't know about...

   By telling the import-operations to use THIS factory instead, we can silence the
   import and even extend it to set ImportOptions programmatically, from command
   values, if we see the need for that.


   How importing works (for reference if I have to look something up again):
   - The import process creates a Factory for the Filetype, checks if there 
     already exists an object where the Factory would create on. If so, dialogs for
	 user response are created.
   - The import process calls UFactory::StaticImportObject and provides the Factory
     with which to create the Object. If no Factory is provided, UFactory will find 
	 one automatically. (For the Content Browser the import process starts in
	 FAssetTools::ImportAssets)
   - Then UFactory does some evaluation about the File from which to import, if the
     provided Factory reads binary or text and so on.
	 It will finally call Factory->FactoryCreateBinary
   - The (Fbx)Factory then will then detect the type to import (StaticMesh or SkelMesh)
     It will create the UI where the User can set Flags.
	 And finally import the File (from UFactory::CurrentFilename) with the FFbxImporter
 */

UCLASS(hidecategories=Object)
class Um2uFbxFactory : public UFbxFactory
{
	GENERATED_UCLASS_BODY()

	// Begin UObject Interface
	//virtual void CleanUp() OVERRIDE;
	virtual bool ConfigureProperties() override;
	//virtual void PostInitProperties() OVERRIDE;
	// End UObject Interface
};
