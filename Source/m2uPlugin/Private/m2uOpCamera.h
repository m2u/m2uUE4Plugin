#pragma once
// Operations on the Viewport Camera

#include "m2uOperation.h"

#include "ActorEditorUtils.h"
#include "UnrealEd.h"
#include "m2uHelper.h"

class Fm2uOpCamera : public Fm2uOperation
{
public:

	Fm2uOpCamera( Fm2uOperationManager* Manager = NULL )
		:Fm2uOperation( Manager ){}

	bool Execute( FString Cmd, FString& Result ) override
	{
		const TCHAR* Str = *Cmd;
		bool DidExecute = true;

		if( FParse::Command(&Str, TEXT("TransformCamera")))
		{
			/* this command is meant for viewports, not for camera Actors */
			const TCHAR* Stream = Str;
			FVector Loc;
			Stream = GetFVECTORSpaceDelimited( Stream, Loc );
			// jump over the space
			Stream = FCString::Strchr(Stream,' ');
			if( Stream != NULL )
			{
				++Stream;
			}
			FRotator Rot;
			Stream = GetFROTATORSpaceDelimited( Stream, Rot, 1.0f );

			if( Stream != NULL )
			{
				for( int32 i=0; i<GEditor->LevelViewportClients.Num(); i++ )
				{
					GEditor->LevelViewportClients[i]->SetViewLocation( Loc );
					GEditor->LevelViewportClients[i]->SetViewRotation( Rot );
				}
			}
			GEditor->RedrawLevelEditingViewports();
		}

		else
		{
			// cannot handle the passed command
			DidExecute = false;
		}

		Result = TEXT("Ok");
		if( DidExecute )
			return true;
		else
			return false;
	}
};
