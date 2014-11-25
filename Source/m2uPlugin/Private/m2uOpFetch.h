#pragma once
// Operations to export/fetch the Scene and its Assets

#include "m2uOperation.h"

#include "ActorEditorUtils.h"
#include "UnrealEd.h"
#include "m2uHelper.h"


class Fm2uOpFastFetch : public Fm2uOperation
{
public:

Fm2uOpFastFetch( Fm2uOperationManager* Manager = NULL )
	:Fm2uOperation( Manager ){}

	bool Execute( FString Cmd, FString& Result ) override
	{
		const TCHAR* Str = *Cmd;
		bool DidExecute = true;

		// fast fetching exports the selected objects simply into an fbx (or obj) file
		if( FParse::Command(&Str, TEXT("FetchSelected")))
		{
			// extract quoted file-path
			const FString FilePath = FParse::Token(Str,0);
			auto World = GEditor->GetEditorWorldContext().World();
			GEditor->ExportMap(World, *FilePath, true);
			Result = TEXT("Ok");
		}

		else
		{
// cannot handle the passed command
			DidExecute = false;
		}

		if( DidExecute )
			return true;
		else
			return false;
	}
};
