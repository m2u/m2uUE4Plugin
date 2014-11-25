#pragma once
// Display Layer Operations

#include "m2uOperation.h"

#include "ActorEditorUtils.h"
#include "UnrealEd.h"
#include "Editor/UnrealEd/Private/Layers/Layers.h"
#include "m2uHelper.h"


class Fm2uOpLayer : public Fm2uOperation
{
public:

Fm2uOpLayer( Fm2uOperationManager* Manager = NULL )
	:Fm2uOperation( Manager ){}

	bool Execute( FString Cmd, FString& Result ) override
	{
		const TCHAR* Str = *Cmd;
		bool DidExecute = true;

		if( FParse::Command(&Str, TEXT("AddObjectsToLayer")))
		{
			FString LayerName = FParse::Token(Str,0);
			FString ActorNamesList = FParse::Token(Str,0);
			bool bRemoveFromOthers = true;
			FParse::Bool(Str, TEXT("RemoveFromOthers="), bRemoveFromOthers);

			UE_LOG(LogM2U, Log, TEXT("AddObjectsToLayer received: %s %s"), *LayerName, *ActorNamesList);

			TArray<FString> ActorNames = m2uHelper::ParseList(ActorNamesList);
			for( FString ActorName : ActorNames )
			{
				UE_LOG(LogM2U, Log, TEXT("Actor Names List: %s"), *ActorName);
				AActor* Actor;
				if( m2uHelper::GetActorByName( *ActorName, &Actor) )
				{
					//if( FCString::Stricmp( *RemoveFromOthers, TEXT("True") )==0 )
					if( bRemoveFromOthers )
					{
						UE_LOG(LogM2U, Log, TEXT("Removing Actor %s from all Others"), *ActorName);
						TArray<FName> AllLayerNames;
						GEditor->Layers->AddAllLayerNamesTo(AllLayerNames);
						GEditor->Layers->RemoveActorFromLayers(Actor, AllLayerNames);
					}
					UE_LOG(LogM2U, Log, TEXT("Adding Actor %s to Layer %s"), *ActorName, *LayerName);
					GEditor->Layers->AddActorToLayer(Actor, FName(*LayerName));
				}
			}
		}

		else if( FParse::Command(&Str, TEXT("RemoveObjectsFromAllLayers")))
		{
			FString ActorNamesList = FParse::Token(Str,0);
			TArray<FString> ActorNames = m2uHelper::ParseList(ActorNamesList);
			for( FString ActorName : ActorNames )
			{
				AActor* Actor;
				if( m2uHelper::GetActorByName( *ActorName, &Actor) )
				{
					UE_LOG(LogM2U, Log, TEXT("Removing Actor %s from all Layers."), *ActorName);
					TArray<FName> AllLayerNames;
					GEditor->Layers->AddAllLayerNamesTo(AllLayerNames);
					GEditor->Layers->RemoveActorFromLayers(Actor, AllLayerNames);
				}
			}
		}

		else if( FParse::Command(&Str, TEXT("HideLayer")))
		{
			FString LayerName = FParse::Token(Str,0);
			GEditor->Layers->SetLayerVisibility(FName(*LayerName), false);
			UE_LOG(LogM2U, Log, TEXT("Hiding Layer: %s"), *LayerName);
		}

		else if( FParse::Command(&Str, TEXT("UnhideLayer")))
		{
			FString LayerName = FParse::Token(Str,0);
			GEditor->Layers->SetLayerVisibility(FName(*LayerName), true);
			UE_LOG(LogM2U, Log, TEXT("Unhiding Layer: %s"), *LayerName);
		}

		else if( FParse::Command(&Str, TEXT("DeleteLayer")))
		{
			FString LayerName = FParse::Token(Str,0);
			GEditor->Layers->DeleteLayer(FName(*LayerName));
			UE_LOG(LogM2U, Log, TEXT("Deleting Layer: %s"), *LayerName);
		}

		else if( FParse::Command(&Str, TEXT("RenameLayer")))
		{
			FString OldName = FParse::Token(Str,0);
			FString NewName = FParse::Token(Str,0);
			GEditor->Layers->RenameLayer(FName(*OldName), FName(*NewName));
			UE_LOG(LogM2U, Log, TEXT("Renaming Layer %s to %s"), *OldName, *NewName);
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
