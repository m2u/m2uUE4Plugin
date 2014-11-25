#pragma once
// Operations for Undo and Redo

#include "m2uOperation.h"

#include "ActorEditorUtils.h"
#include "UnrealEd.h"
#include "m2uHelper.h"


class Fm2uOpTransaction : public Fm2uOperation
{
public:

Fm2uOpTransaction( Fm2uOperationManager* Manager = NULL )
	:Fm2uOperation( Manager ){}

	bool Execute( FString Cmd, FString& Result ) override
	{
		const TCHAR* Str = *Cmd;
		bool DidExecute = true;

		if( FParse::Command(&Str, TEXT("Undo")))
		{
			GEditor->UndoTransaction();
			Result = TEXT("Ok");
		}

		else if( FParse::Command(&Str, TEXT("Redo")))
		{
			GEditor->RedoTransaction();
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
