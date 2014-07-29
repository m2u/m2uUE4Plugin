
#ifndef _M2UACTIONS_H_
#define _M2UACTIONS_H_

#include "m2uHelper.h"

/*
  This file is only a temporary collection of actions that are required by not
  only one direct command in the ExecuteCommand function.

  This will all be refactored and this file be removed once I go Object-Oriented
  with the Actions.
*/
namespace m2uActions
{

	using namespace m2uHelper;

	FString TransformObject(const TCHAR* Str)
	{
		const FString ActorName = FParse::Token(Str,0);
		AActor* Actor = NULL;
		UE_LOG(LogM2U, Log, TEXT("Searching for Actor with name %s"), *ActorName);

		if(!GetActorByName(*ActorName, &Actor) || Actor == NULL)
		{
			UE_LOG(LogM2U, Log, TEXT("Actor %s not found or invalid."), *ActorName);
			return TEXT("1");
		}

		m2uHelper::SetActorTransformRelativeFromText(Actor, Str);

		GEditor->RedrawLevelEditingViewports();
		return TEXT("Ok");
	}


	FString ParentChildTo(const TCHAR* Str)
	{
		const FString ChildName = FParse::Token(Str,0);
		Str = FCString::Strchr(Str,' ');
		FString ParentName;
		if( Str != NULL) // there may be a parent name present
		{
			Str++;
			if( *Str != '\0' ) // there was a space, but no name after that
			{
				ParentName = FParse::Token(Str,0);
			}
		}

		AActor* ChildActor = NULL;
		if(!GetActorByName(*ChildName, &ChildActor) || ChildActor == NULL)
		{
			UE_LOG(LogM2U, Log, TEXT("Actor %s not found or invalid."), *ChildName);
			return TEXT("1");
		}

		// TODO: enable transaction?
		//const FScopedTransaction Transaction( NSLOCTEXT("Editor", "UndoAction_PerformAttachment", "Attach actors") );

		// parent to world, aka "detach"
		if( ParentName.Len() < 1) // no valid parent name
		{
			USceneComponent* ChildRoot = ChildActor->GetRootComponent();
			if(ChildRoot->AttachParent != NULL)
			{
				UE_LOG(LogM2U, Log, TEXT("Parenting %s the World."), *ChildName);
				AActor* OldParentActor = ChildRoot->AttachParent->GetOwner();
				OldParentActor->Modify();
				ChildRoot->DetachFromParent(true);
				//ChildActor->SetFolderPath(OldParentActor->GetFolderPath());

				GEngine->BroadcastLevelActorDetached(ChildActor, OldParentActor);
			}
			return TEXT("0");
		}

		AActor* ParentActor = NULL;
		if(!GetActorByName(*ParentName, &ParentActor) || ParentActor == NULL)
		{
			UE_LOG(LogM2U, Log, TEXT("Actor %s not found or invalid."), *ParentName);
			return TEXT("1");
		}
		if( ParentActor == ChildActor ) // can't parent actor to itself
		{
			return TEXT("1");
		}
		// parent to other actor, aka "attach"
		UE_LOG(LogM2U, Log, TEXT("Parenting %s to %s."), *ChildName, *ParentName);
		GEditor->ParentActors( ParentActor, ChildActor, NAME_None);

		return TEXT("0");
	}

	/**
	   parses a string to interpret what actor to add and what properties to set
	   currently only supports name and transform properties
	   TODO: add support for other actors like lights and so on
	 */
	FString AddActor(const TCHAR* Str)
	{
		FString AssetName = FParse::Token(Str,0);
		const FString ActorName = FParse::Token(Str,0);
		auto World = GEditor->GetEditorWorldContext().World();
		ULevel* Level = World->GetCurrentLevel();

		FName ActorFName = m2uHelper::GetFreeName(ActorName);
		AActor* Actor = m2uHelper::AddNewActorFromAsset(AssetName, Level, ActorFName, false);
		if( Actor == NULL )
		{
			//UE_LOG(LogM2U, Log, TEXT("failed creating from asset"));
			return TEXT("1");
		}

		ActorFName = Actor->GetFName();

		// now we might have transformation data in that string 
		// so set that, while we already have that actor 
		// (no need in searching it again later
		m2uHelper::SetActorTransformRelativeFromText(Actor, Str);

		// TODO: we might have other property data in that string
		// we need a function to set light radius and all that

		return ActorFName.ToString();
	}

	/**
	   add multiple actors from the string,
	   expects every line to be a new actor
	 */
	FString AddActorBatch(const TCHAR* Str)
	{
		UE_LOG(LogM2U, Log, TEXT("Batch Add parsing lines"));
		FString Line;
		while( FParse::Line(&Str, Line, 0) )
		{
			if( Line.IsEmpty() )
				continue;
			UE_LOG(LogM2U, Log, TEXT("Read one Line: %s"),*Line);
			AddActor(*Line);
		}
		return TEXT("Ok");
	}


	/**
	   will import all assets listed after the destination path into the destination
	   path. It will not recreate folder structures, only import the files directly
	   into the destination.
	   If you specify a folder instead of a file, all files in that folder and
	   its subfolders will be imported, recreating the folder structure with
	   the specified folder being at the same level as the destination path.
	   
	 */
	FString ImportAssets(const TCHAR* Str)
	{
		FString RootDestinationPath = FParse::Token(Str,0);
		TArray<FString> Files;
		FString AssetFile;
		while( FParse::Token(Str, AssetFile, 0) )
		{
			Files.Add(AssetFile);
		}
		m2uHelper::ImportAssets(Files, RootDestinationPath, false/*, &GetUserInput*/ );
		return TEXT("Ok");
	}

	/**
	   Will import all assets listed in the string to its assocated destination
	   so the string must always contain "/DestinationPath" "/FilePath"
	   separated by whitespaces for each file to import.
	   The DestinationPath must not contain the AssetName!
	   Use this to mirror a folder structure by explicitly telling the Editor
	   to create the destination path for each source file.

	   Of course if one of the specified AssetSource values is a Folder, all files
	   and subfolders will be imported.
	*/
	FString ImportAssetsBatch(const TCHAR* Str)
	{
		FString AssetDestination;
		FString AssetSource;
		while( FParse::Token(Str, AssetDestination, 0) )
		{
			if( FParse::Token(Str, AssetSource, 0) )
			{
				TArray<FString> Files;
				Files.Add(AssetSource);
				m2uHelper::ImportAssets(Files, AssetDestination, false/*, &GetUserInput*/); 
			}
			else // there is an uneven list of Destination<->FilePath infos
			{
				// TODO: maybe store all associations in a pair-list first,
				// and start the import process only if they are even. Because
				// the reason for unevenness may be earlier than the end of the list.
				// We would import crap then maybe or create invalid destination paths
				// which in turn might crash the editor.
				UE_LOG(LogM2U, Error, TEXT("Uneven list of Destination<->FilePath infos for Import."));
				return TEXT("1");
			}
		}
		return TEXT("Ok");		
	}
}// namespace m2uActions

#endif /* _M2UACTIONS_H_ */
