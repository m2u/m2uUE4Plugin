#pragma once

#include "m2uOperation.h"


/**
 * Tests for message interpretation and communication.
 *
 * A client-side test should call these commands and make sure the
 * return value is correct.
 */
class Fm2uOpTests : public Fm2uOperation
{
public:

	Fm2uOpTests( Fm2uOperationManager* Manager = NULL )
		:Fm2uOperation( Manager ){}

	bool Execute( FString Cmd, FString& Result ) override
	{
		const TCHAR* Str = *Cmd;

		// Test that a message is read completely. Return the length
		// of the message as result text.  This should match with the
		// length on the client's side.
		if (FParse::Command(&Str, TEXT("TestMessageSize")))
		{
			Result = FString::Printf(TEXT("%i"), Cmd.Len());
			return true;
		}

		return false;
	}
};
