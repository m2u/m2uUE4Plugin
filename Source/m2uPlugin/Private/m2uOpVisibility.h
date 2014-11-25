#pragma once
// Visibility Operations

#include "m2uOperation.h"

#include "ActorEditorUtils.h"
#include "UnrealEd.h"
#include "m2uHelper.h"

class Fm2uOpVisibility : public Fm2uOperation
{
public:

Fm2uOpVisibility( Fm2uOperationManager* Manager = NULL )
		:Fm2uOperation( Manager ){}

	bool Execute( FString Cmd, FString& Result ) override
	{
		const TCHAR* Str = *Cmd;
		bool DidExecute = true;

		// also see the "edactHide..." functions
		// these functions currently do not write to the TransactionBuffer, 
		// so won't be undoable
		if( FParse::Command(&Str, TEXT("HideSelected")))
		{
			TArray<AActor*> SelectedActors;
			USelection* Selection = GEditor->GetSelectedActors();
			Selection->Modify();
			Selection->GetSelectedObjects<AActor>(SelectedActors);

			for( int32 Idx = 0 ; Idx < SelectedActors.Num() ; ++Idx )
			{
				AActor* Actor = SelectedActors[ Idx ];
				// Don't consider already hidden actors or the builder brush
				if( !FActorEditorUtils::IsABuilderBrush(Actor) && !Actor->IsHiddenEd() )
				{
					Actor->SetIsTemporarilyHiddenInEditor( true );
				}
			}
			GEditor->RedrawLevelEditingViewports();
		}

		else if( FParse::Command(&Str, TEXT("UnhideSelected")))
		{
			TArray<AActor*> SelectedActors;
			USelection* Selection = GEditor->GetSelectedActors();
			Selection->Modify();
			Selection->GetSelectedObjects<AActor>(SelectedActors);

			for( int32 Idx = 0 ; Idx < SelectedActors.Num() ; ++Idx )
			{
				AActor* Actor = SelectedActors[ Idx ];
				// Don't consider already visible actors or the builder brush
				if( !FActorEditorUtils::IsABuilderBrush(Actor) && Actor->IsHiddenEd() )
				{
					Actor->SetIsTemporarilyHiddenInEditor( false );
				}
			}
			GEditor->RedrawLevelEditingViewports();
		}

		else if( FParse::Command(&Str, TEXT("IsolateSelected")))
		{
			// Iterate through all of the actors and hide the ones which
			// are not selected and are not already hidden
			auto World = GEditor->GetEditorWorldContext().World();
			for( FActorIterator It(World); It; ++It )
			{
				AActor* Actor = *It;
				if( !FActorEditorUtils::IsABuilderBrush(Actor) && !Actor->IsSelected() && !Actor->IsHiddenEd() )
				{
					Actor->SetIsTemporarilyHiddenInEditor( true );
				}
			}
			GEditor->RedrawLevelEditingViewports();
		}

		else if( FParse::Command(&Str, TEXT("UnhideAll")))
		{
			// Iterate through all of the actors and unhide them
			auto World = GEditor->GetEditorWorldContext().World();
			for( FActorIterator It(World); It; ++It )
			{
				AActor* Actor = *It;
				if( !FActorEditorUtils::IsABuilderBrush(Actor) && Actor->IsTemporarilyHiddenInEditor() )
				{
					Actor->SetIsTemporarilyHiddenInEditor( false );
				}
			}
			GEditor->RedrawLevelEditingViewports();
		}

		else if( FParse::Command(&Str, TEXT("HideByNames")))
		{
			FString Name;
			while( FParse::Token(Str, Name, 0) )
			{
				AActor* Actor = NULL;
				if(m2uHelper::GetActorByName(*Name, &Actor) && !Actor->IsHiddenEd())
				{
					Actor->SetIsTemporarilyHiddenInEditor( true );
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
