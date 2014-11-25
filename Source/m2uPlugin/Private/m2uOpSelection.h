#pragma once
// Selection Operations

#include "m2uOperation.h"

#include "ActorEditorUtils.h"
#include "UnrealEd.h"

class Fm2uOpSelection : public Fm2uOperation
{
public:

Fm2uOpSelection( Fm2uOperationManager* Manager = NULL )
		:Fm2uOperation( Manager ){}

	bool Execute( FString Cmd, FString& Result ) override
	{
		const TCHAR* Str = *Cmd;
		bool DidExecute = false;

		if( FParse::Command(&Str, TEXT("SelectByName")))
		{
			const FString ActorName = FParse::Token(Str,0);
			UE_LOG(LogM2U, Log, TEXT("received SelectByName: %s."), 
					   *ActorName);
			//AActor* Actor = GEditor->SelectNamedActor(*ActorName);
			AActor* Actor = NULL;
			if( m2uHelper::GetActorByName(*ActorName, Actor) )
			{
				GEditor->SelectActor( Actor, true, true, true);// actor, select, notify, evenIfHidden
				GEditor->RedrawLevelEditingViewports();
			}
			else
			{
				UE_LOG(LogM2U, Log, TEXT("Actor %s not found or not selectable."), 
					   *ActorName);
			}

			DidExecute = true;
		}
		else if( FParse::Command(&Str, TEXT("DeselectAll")))
		{
			GEditor->SelectNone(true, true, false);
			GEditor->RedrawLevelEditingViewports();

			DidExecute = true;
		}
		else if( FParse::Command(&Str, TEXT("DeselectByName")))
		{
			const FString ActorName = FParse::Token(Str,0);
			TArray<AActor*> SelectedActors;
			USelection* Selection = GEditor->GetSelectedActors();
			Selection->GetSelectedObjects<AActor>(SelectedActors);

			for( int32 Idx = 0 ; Idx < SelectedActors.Num() ; ++Idx )
			{
				AActor* Actor = SelectedActors[ Idx ];
				if(Actor->GetFName().ToString() == ActorName)
				{
					Selection->Modify();
					//Selection->BeginBatchSelectOperation();
					//Selection->Deselect(Actor);
					//Selection->EndBatchSelectOperation();
					GEditor->SelectActor( Actor, false, false ); // deselect
					break;
				}
			}
			GEditor->RedrawLevelEditingViewports();
			DidExecute = true;
		}

		Result = TEXT("Ok");
		if( DidExecute )
			return true;
		else
			return false;
	}
};
