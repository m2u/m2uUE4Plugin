// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "m2uPluginPrivatePCH.h"
#include "m2uPlugin.generated.inl"

#include "Networking.h"
#include "ActorEditorUtils.h"
#include "UnrealEd.h"
#include "Editor/UnrealEd/Private/Layers/Layers.h"
#include "Runtime/Engine/Classes/Engine/TextRenderActor.h"

#include "m2uHelper.h"
#include "m2uActions.h"
#include "m2uBatchFileParse.h"

DEFINE_LOG_CATEGORY( LogM2U )

#define DEFAULT_M2U_ENDPOINT FIPv4Endpoint(FIPv4Address(127,0,0,1), 3939)

IMPLEMENT_MODULE( Fm2uPlugin, m2uPlugin )


//bool GetActorByName( const TCHAR* Name, AActor* OutActor, UWorld* InWorld);
FString ExecuteCommand(const TCHAR* Str/*, Fm2uPlugin* Conn*/);

Fm2uPlugin::Fm2uPlugin()
	:TcpListener(NULL),
	 Client(NULL)
{
}

void Fm2uPlugin::StartupModule()
{
    // This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
	UE_LOG(LogM2U, Log, TEXT("starting up m2u"));
	if( !GIsEditor )
	{
		return;
	}

	TcpListener = new FTcpListener( DEFAULT_M2U_ENDPOINT );
	TcpListener->OnConnectionAccepted().BindRaw(this, &Fm2uPlugin::HandleConnectionAccepted);

	TickObject = new Fm2uTickObject(this);

}


void Fm2uPlugin::ShutdownModule()
{

    // close all clients
	if(Client != NULL)
	{
		Client->Close();
		Client=NULL;
	}

	TcpListener->Stop();
	delete TcpListener;
	TcpListener = NULL;

	delete TickObject;
	TickObject = NULL;
}

bool Fm2uPlugin::Exec( UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar )
{
	if( FParse::Command(&Cmd, TEXT("m2uCloseConnection")) )
	{
		if(Client != NULL)
		{
			Client->Close();
			Client=NULL;
		}
		return true;
	}
	else if( FParse::Command(&Cmd, TEXT("m2uBatchFileParse")) )
	{
		FString Filename;
		if( FParse::Token(Cmd, Filename, 0))
		{
			m2uBatchFileParse(Filename);
		}
		return true;
	}
	else if( FParse::Command(&Cmd, TEXT("m2uDo")) )
	{
		// execute an Action without using tcp connection
		ExecuteCommand(Cmd);
		return true;
	}
	return false;
}

bool Fm2uPlugin::HandleConnectionAccepted( FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint)
{
	if(Client==NULL)
	{
		Client = ClientSocket;
		UE_LOG(LogM2U, Log, TEXT("connected on port %i"),Client->GetPortNo());
		return true;
	}
	UE_LOG(LogM2U, Log, TEXT("connection declined"));
	return false;
}


void Fm2uPlugin::Tick( float DeltaTime )
{
	// valid and connected?
	if( Client != NULL && Client -> GetConnectionState() == SCS_Connected)
	{
		//UE_LOG(LogM2U, Log, TEXT("Tick time was %f"),DeltaTime);
		// get the message, do stuff, and tell the caller what happened ;)
		FString Message;	
		if( GetMessage(Message) )
		{
			FString Result = ExecuteCommand(*Message);
			SendResponse(Result);
		}
	}
}


bool Fm2uPlugin::GetMessage(FString& Result)
{
	// get all data from client and create one long FString from it
	// return the result when the client is out of pending data.
	//FString Result;
	uint32 DataSize = 0;
	while(Client->HasPendingData(DataSize) && DataSize > 0 )
	{
		//UE_LOG(LogM2U, Log, TEXT("pending data size %i"), DataSize);
        // create data array to read from client
		//FArrayReaderPtr Data = MakeShareable(new FArrayReader(true));
		FArrayReader Data;
		Data.Init(DataSize);

		int32 BytesRead = 0;
        // read pending data into the Data array reader
		if( Client->Recv( Data.GetData(), Data.Num(), BytesRead) )
		{
			//UE_LOG(LogM2U, Log, TEXT("DataNum %i, BytesRead: %i"), Data.Num(), BytesRead);

			// the data we receive is supposed to be ansi, but we will work with TCHAR, so we have to convert
			int32 DestLen = TStringConvert<ANSICHAR,TCHAR>::ConvertedLength((char*)(Data.GetData()),Data.Num());
			//UE_LOG(LogM2U, Log, TEXT("DestLen will be %i"), DestLen);
			TCHAR* Dest = new TCHAR[DestLen+1];
			TStringConvert<ANSICHAR,TCHAR>::Convert(Dest, DestLen, (char*)(Data.GetData()), Data.Num());
			Dest[DestLen]='\0';

			//FString Text(Dest); // FString from tchar array
			//UE_LOG(LogM2U, Log, TEXT("server received %s"), *Text);
			//UE_LOG(LogM2U, Log, TEXT("Server received: %s"), Dest);

			//FString Result = ExecuteCommand(Dest, this);
			//SendResponse(Result);

			// add all the message parts to the Result
			Result += Dest;

			delete Dest;
		}
	}// while
	if(! Result.IsEmpty() )
		return true;
	else
		return false;
}

void Fm2uPlugin::SendResponse(const FString& Message)
{
	if( Client != NULL && Client -> GetConnectionState() == SCS_Connected)
	{
		//const uint8* Data = *Message;
		//const int32 Count = Message.Len();
		int32 DestLen = TStringConvert<TCHAR,ANSICHAR>::ConvertedLength(*Message, Message.Len());
		//UE_LOG(LogM2U, Log, TEXT("DestLen will be %i"), DestLen);
		uint8* Dest = new uint8[DestLen+1];
		TStringConvert<TCHAR,ANSICHAR>::Convert((ANSICHAR*)Dest, DestLen, *Message, Message.Len());
		Dest[DestLen]='\0';

		int32 BytesSent = 0;
		if(	! Client->Send( Dest, DestLen, BytesSent) )
		{
			UE_LOG(LogM2U, Error, TEXT("TCP Server sending answer failed."));
		}
	}
}

//void HandleReceivedData(FArrayReader& Data)

// TODO: this is only temporaryly here until we go full Oject-Oriented and so
FString GetUserInput(const FString& Problem)
{
	// TODO: get the m2uPlugin instance here from the plugin-manager to send messages
	// over TCP for user input.
	UE_LOG(LogM2U, Log, TEXT("GetUserInput was called"));
	if( Problem.StartsWith(TEXT("UsedByMap")) )
	{
		return TEXT("Skip"); 
	}
	else if( Problem.StartsWith(TEXT("Overwrite")) )
	{
		return TEXT("YesAll");
	}
	else if( Problem.StartsWith(TEXT("Replace")) )
	{
		return TEXT("SkipAll");
	}
	UE_LOG(LogM2U, Log, TEXT("Unknown Problem: %s"), *Problem);
	return TEXT("");
	
}

FString ExecuteCommand(const TCHAR* Str/*, Fm2uPlugin* Conn*/)
{
	//UE_LOG(LogM2U, Log, TEXT("Executing Command: %s"), Str);
	if( FParse::Command(&Str, TEXT("Exec")))
	{
		if( GEditor->Exec(GEditor->GetEditorWorldContext().World(), Str) )
			return TEXT("Ok");
		else
			return TEXT("Command unhandled.");
	}

	else if( FParse::Command(&Str, TEXT("Test")))
	{
		return TEXT("Hallo");
	}

	// -- SELECTION --
	else if( FParse::Command(&Str, TEXT("SelectByName")))
	{
		const FString ActorName = FParse::Token(Str,0);
		AActor* Actor = GEditor->SelectNamedActor(*ActorName);
		GEditor->RedrawLevelEditingViewports();

		return TEXT("Ok");
	}
	else if( FParse::Command(&Str, TEXT("DeselectAll")))
	{
		GEditor->SelectNone(true, true, false);
		GEditor->RedrawLevelEditingViewports();

		return TEXT("Ok");
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
		return TEXT("Ok");
	}


	// -- EDITING --
	else if( FParse::Command(&Str, TEXT("TransformObject")))
	{
		return m2uActions::TransformObject(Str);
	}
	else if( FParse::Command(&Str, TEXT("DeleteSelected")))
	{
		auto World = GEditor->GetEditorWorldContext().World();
		((UUnrealEdEngine*)GEditor)->edactDeleteSelected(World);

		return TEXT("Ok");
	}
	else if( FParse::Command(&Str, TEXT("DeleteObject")))
	{
		// deletion of actors in the editor is a dangerous/complex task as actors
		// can be brushes or referenced, levels need to be dirtied and so on
		// there are no "deleteActor" functions in the Editor, only "DeleteSelected"
		// since in most cases a deletion is preceded by a selection, and followed by
		// a selection change, we don't bother and just select the object to delete
		// and use the editor function to do it.
		// TODO: maybe we could reselect the previous selection after the delete op
		// but this is probably in 99% of the cases not necessary
		GEditor->SelectNone(true, true, false);
		const FString ActorName = FParse::Token(Str,0);
		AActor* Actor = GEditor->SelectNamedActor(*ActorName);
		auto World = GEditor->GetEditorWorldContext().World();
		((UUnrealEdEngine*)GEditor)->edactDeleteSelected(World);
		
		return TEXT("Ok");
	}
	else if( FParse::Command(&Str, TEXT("RenameObject")))
	{
		const FString ActorName = FParse::Token(Str,0);
		// jump over the next space
		Str = FCString::Strchr(Str,' ');
		if( Str != NULL)
			Str++;
		// the desired new name
		const FString NewName = FParse::Token(Str,0);

		// find the Actor
		AActor* Actor = NULL;
		if(!m2uHelper::GetActorByName(*ActorName, &Actor) || Actor == NULL)
		{
			UE_LOG(LogM2U, Log, TEXT("Actor %s not found or invalid."), *ActorName);
			return TEXT("1"); // NOT FOUND
		}

		// try to rename the actor
		const FName ResultName = m2uHelper::RenameActor(Actor, NewName);
		return ResultName.ToString();
	}
	else if( FParse::Command(&Str, TEXT("DuplicateObject")))
	{
		const FString ActorName = FParse::Token(Str,0);
		AActor* OrigActor = NULL;
		AActor* Actor = NULL; // the duplicate
		//UE_LOG(LogM2U, Log, TEXT("Searching for Actor with name %s"), *ActorName);

		// Find the Original to clone
		if(!m2uHelper::GetActorByName(*ActorName, &OrigActor) || OrigActor == NULL)
		{
			UE_LOG(LogM2U, Log, TEXT("Actor %s not found or invalid."), *ActorName);
			return TEXT("1"); // original not found
		}

		// jump over the next space to find the name for the Duplicate
		Str = FCString::Strchr(Str,' ');
		if( Str != NULL)
			Str++;

		// the name that is desired for the object
		const FString DupName = FParse::Token(Str,0);

		// TODO: enable transactions
		//const FScopedTransaction Transaction( NSLOCTEXT("UnrealEd", "DuplicateActors", "Duplicate Actors") );

		// select only the actor we want to duplicate
		GEditor->SelectNone(true, true, false);
		//OrigActor = GEditor->SelectNamedActor(*ActorName); // actor to duplicate
		GEditor->SelectActor(OrigActor, true, false);
		auto World = GEditor->GetEditorWorldContext().World();
		// Do the duplication
		((UUnrealEdEngine*)GEditor)->edactDuplicateSelected(World->GetCurrentLevel(), false);

		// get the new actor (it will be auto-selected by the editor)
		FSelectionIterator It( GEditor->GetSelectedActorIterator() );
		Actor = static_cast<AActor*>( *It );

		if( ! Actor )
			return TEXT("4"); // duplication failed?

		// if there are transform parameters in the command, apply them
		m2uHelper::SetActorTransformRelativeFromText(Actor, Str);

		GEditor->RedrawLevelEditingViewports();

		// Try to set the actor's name to DupName
		// NOTE: a unique name was already assigned during the actual duplicate
		// operation, we could just return that name instead and say "the editor
		// changed the name" but if the DupName can be taken, it will save a lot of
		// extra work on the program side which has to find a new name otherwise.
		//GEditor->SetActorLabelUnique( Actor, DupName );
		m2uHelper::RenameActor(Actor, DupName);

		// get the editor-set name
		const FString AssignedName = Actor->GetFName().ToString();
		// if it is the desired name, everything went fine, if not,
		// send the name as a response to the caller
		if( AssignedName == DupName )
		{
			//Conn->SendResponse(TEXT("0"));
			return TEXT("0");
		}
		else
		{
			//Conn->SendResponse(FString::Printf( TEXT("3 %s"), *AssignedName ) );
			return FString::Printf( TEXT("3 %s"), *AssignedName );
		}

	}

	else if( FParse::Command(&Str, TEXT("ParentChildTo")))
	{
		return m2uActions::ParentChildTo(Str);
	}



	// -- VISIBILITY --
	// also see the "edactHide..." functions
	// these functions currently do not write to the TransactionBuffer, so won't be undoable
	else if( FParse::Command(&Str, TEXT("HideSelected")))
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
		return TEXT("Ok");
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
		return TEXT("Ok");
	}
	else if( FParse::Command(&Str, TEXT("IsolateSelected")))
	{
		// Iterate through all of the actors and hide the ones which are not selected and are not already hidden
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
		return TEXT("Ok");
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
		return TEXT("Ok");
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
		return TEXT("Ok");
	}


	// -- CAMERA --
	else if( FParse::Command(&Str, TEXT("TransformCamera")))
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
		return TEXT("Ok");
	}

	// -- OTHER --
	else if( FParse::Command(&Str, TEXT("Undo")))
	{
		GEditor->UndoTransaction();
		return TEXT("Ok");
	}
	else if( FParse::Command(&Str, TEXT("Redo")))
	{
		GEditor->RedoTransaction();
		return TEXT("Ok");
	}
	else if( FParse::Command(&Str, TEXT("GetFreeName")))
	{
		const FString InName = FParse::Token(Str,0);
		FName FreeName = m2uHelper::GetFreeName(InName);
		return FreeName.ToString();
	}


	// -- EXPORTING --

	// fast fetching exports the selected objects simply into an fbx (or obj) file
	else if( FParse::Command(&Str, TEXT("FetchSelected")))
	{
		// extract quoted file-path
		const FString FilePath = FParse::Token(Str,0);
		auto World = GEditor->GetEditorWorldContext().World();
		GEditor->ExportMap(World, *FilePath, true);
		return TEXT("Ok");
	}


	else if( FParse::Command(&Str, TEXT("AddActor")))
	{
		return m2uActions::AddActor(Str);
	}
	else if( FParse::Command(&Str, TEXT("AddActorBatch")))
	{
		return m2uActions::AddActorBatch(Str);
	}

	else if( FParse::Command(&Str, TEXT("ImportAssets")))
	{
		return m2uActions::ImportAssets(Str);
	}
	else if( FParse::Command(&Str, TEXT("ImportAssetsBatch")))
	{
		return m2uActions::ImportAssetsBatch(Str);
	}

	else if( FParse::Command(&Str, TEXT("ExportAsset")))
	{
		UE_LOG(LogM2U, Log, TEXT("Received ExportAsset: %s"), Str);
		FString AssetName = FParse::Token(Str,0);
		FString ExportPath = FParse::Token(Str,0);
		m2uHelper::ExportAsset(AssetName, ExportPath);
		return TEXT("Ok");
	}

	else if( FParse::Command(&Str, TEXT("AddObjectsToLayer")))
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
		return TEXT("Ok");
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
		return TEXT("Ok");
	}
	else if( FParse::Command(&Str, TEXT("HideLayer")))
	{
		FString LayerName = FParse::Token(Str,0);
		GEditor->Layers->SetLayerVisibility(FName(*LayerName), false);
		UE_LOG(LogM2U, Log, TEXT("Hiding Layer: %s"), *LayerName);
		return TEXT("Ok");
	}
	else if( FParse::Command(&Str, TEXT("UnhideLayer")))
	{
		FString LayerName = FParse::Token(Str,0);
		GEditor->Layers->SetLayerVisibility(FName(*LayerName), true);
		UE_LOG(LogM2U, Log, TEXT("Unhiding Layer: %s"), *LayerName);
		return TEXT("Ok");
	}
	else if( FParse::Command(&Str, TEXT("DeleteLayer")))
	{
		FString LayerName = FParse::Token(Str,0);
		GEditor->Layers->DeleteLayer(FName(*LayerName));
		UE_LOG(LogM2U, Log, TEXT("Deleting Layer: %s"), *LayerName);
		return TEXT("Ok");
	}
	else if( FParse::Command(&Str, TEXT("RenameLayer")))
	{
		FString OldName = FParse::Token(Str,0);
		FString NewName = FParse::Token(Str,0);
		GEditor->Layers->RenameLayer(FName(*OldName), FName(*NewName));
		UE_LOG(LogM2U, Log, TEXT("Renaming Layer %s to %s"), *OldName, *NewName);
		return TEXT("Ok");
	}

	else if( FParse::Command(&Str, TEXT("TestActor")))
	{
		auto World = GEditor->GetEditorWorldContext().World();
		ULevel* Level = World->GetCurrentLevel();
		AActor* Actor;
		Actor = m2uHelper::AddNewActorFromAsset(
			FString( TEXT("/Script/Engine.TextRenderActor")), Level,
			FName(TEXT("GroupTransform")));
		Actor->GetRootComponent()->Mobility = EComponentMobility::Static;
		((ATextRenderActor*)Actor) -> TextRender -> SetText(FString(TEXT("Invisible Group Transform")));
		Actor->SetActorHiddenInGame(true);
		return TEXT("Ok");
			
	}

	else if( FParse::Command(&Str, TEXT("LongTest")))
	{
		UE_LOG(LogM2U, Log, TEXT("Received long message: %s"), Str);
		return TEXT("Ok");
	}

	else
	{
		UE_LOG(LogM2U, Warning, TEXT("Command not found: %s"), Str);
		return TEXT("Command Not Found");
	}

}
