#pragma once
// Selection Operations

#include "ActorEditorUtils.h"
#include "UnrealEd.h"

#include "m2uOperation.h"


class Fm2uOpSelection : public Fm2uOperation
{
public:

	Fm2uOpSelection(Fm2uOperationManager* Manager=nullptr)
		:Fm2uOperation( Manager ){}

	bool Execute( FString Cmd, FString& Result ) override
	{
		const TCHAR* Str = *Cmd;
		bool DidExecute = false;

		if (FParse::Command(&Str, TEXT("SelectByNames")))
		{
			FString ActorNamesList = FParse::Token(Str, 0);
			TArray<FString> ActorNames = m2uHelper::ParseList(ActorNamesList);
			for (FString ActorName : ActorNames)
			{
				AActor* Actor;
				if (m2uHelper::GetActorByName(*ActorName, &Actor))
				{
					GEditor->SelectActor(Actor,
					                     /*select=*/true,
					                     /*notify=*/false,
					                     /*evenIfHidden=*/true);
				}
			}
			GEditor->NoteSelectionChange();
			GEditor->RedrawLevelEditingViewports();

			DidExecute = true;
		}

		else if (FParse::Command(&Str, TEXT("DeselectAll")))
		{
			GEditor->SelectNone(true, true, false);
			GEditor->RedrawLevelEditingViewports();

			DidExecute = true;
		}

		else if (FParse::Command(&Str, TEXT("DeselectByNames")))
		{
			TArray<AActor*> SelectedActors;
			USelection* Selection = GEditor->GetSelectedActors();
			Selection->GetSelectedObjects<AActor>(SelectedActors);

			FString ActorNamesList = FParse::Token(Str, 0);
			TArray<FString> ActorNames = m2uHelper::ParseList(ActorNamesList);
			for (FString ActorName : ActorNames)
			{
				for (int32 Idx = 0 ; Idx < SelectedActors.Num() ; ++Idx)
				{
					AActor* Actor = SelectedActors[ Idx ];
					if (Actor->GetFName().ToString() == ActorName)
					{
						Selection->Modify();
						GEditor->SelectActor(Actor,
						                     /*select=*/false,
						                     /*notify=*/false,
						                     /*evenIfHidden=*/true);
						break;
					}
				}
			}
			GEditor->NoteSelectionChange();
			GEditor->RedrawLevelEditingViewports();

			DidExecute = true;
		}

		Result = TEXT("Ok");
		return DidExecute;
	}
};
