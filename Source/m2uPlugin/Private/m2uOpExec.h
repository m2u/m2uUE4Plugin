#pragma once
// Exec Operation allows to execute Editor Exec-commands sent through m2u
#include "m2uOperation.h"

#include "UnrealEd.h"

class Fm2uOpExec : public Fm2uOperation
{
public:

	Fm2uOpExec( Fm2uOperationManager* Manager = NULL )
		:Fm2uOperation( Manager ){}

	bool Execute( FString Cmd, FString& Result ) override
	{
		const TCHAR* Str = *Cmd;
		if( FParse::Command(&Str, TEXT("Exec")))
		{
			if( GEditor->Exec(GEditor->GetEditorWorldContext().World(), Str) )
				Result = TEXT("Ok");		   
			else
				Result = TEXT("Exec-Command unhandled.");

			return true;
		}
		return false;
	}
};
