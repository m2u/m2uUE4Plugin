#pragma once
// A simple test Operation

#include "m2uOperation.h"

class Fm2uOpHelloWorld : public Fm2uOperation
{
public:

	Fm2uOpHelloWorld( Fm2uOperationManager* Manager = NULL )
		:Fm2uOperation( Manager ){}

	bool Execute( FString Cmd, FString& Result ) override
	{
		const TCHAR* Str = *Cmd;
		if( FParse::Command(&Str, TEXT("HelloWorld")))
		{
			Result = TEXT("Hellow World");
			UE_LOG(LogM2U, Log, TEXT("Hello World indeed."));
			return true;
		}
		return false;
	}
};
