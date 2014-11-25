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

		if( FParse::Command(&Str, TEXT("SelectByNames")))
		{
			FString ActorNamesList = FParse::Token(Str,0);
			TArray<FString> ActorNames = m2uHelper::ParseList(ActorNamesList);			
			for( FString ActorName : ActorNames )
			{
				AActor* Actor;
				if( m2uHelper::GetActorByName(*ActorName, &Actor) )
				{
					GEditor->SelectActor( Actor, true, true, true);// actor, select, notify, evenIfHidden
				}			   
			}
			GEditor->RedrawLevelEditingViewports();
			DidExecute = true;
		}

		else if( FParse::Command(&Str, TEXT("DeselectAll")))
		{
			GEditor->SelectNone(true, true, false);
			GEditor->RedrawLevelEditingViewports();

			DidExecute = true;
		}

		else if( FParse::Command(&Str, TEXT("DeselectByNames")))
		{
			TArray<AActor*> SelectedActors;
			USelection* Selection = GEditor->GetSelectedActors();
			Selection->GetSelectedObjects<AActor>(SelectedActors);
			
			FString ActorNamesList = FParse::Token(Str,0);
			TArray<FString> ActorNames = m2uHelper::ParseList(ActorNamesList);			
			for( FString ActorName : ActorNames )
			{
				for( int32 Idx = 0 ; Idx < SelectedActors.Num() ; ++Idx )
				{
					AActor* Actor = SelectedActors[ Idx ];
					if(Actor->GetFName().ToString() == ActorName)
					{
						Selection->Modify();
						//Selection->BeginBatchSelectOperation();
						//Selection->Deselect(Actor);
						//Selection->EndBatchSelectOperation();
						GEditor->SelectActor( Actor, false, true, true ); // deselect
						break;
					}
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
