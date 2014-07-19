// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "m2uPluginPrivatePCH.h"
#include "Networking.h"
#include "ActorEditorUtils.h"
#include "UnrealEd.h"

DEFINE_LOG_CATEGORY( LogM2U )

#define DEFAULT_M2U_ENDPOINT FIPv4Endpoint(FIPv4Address(127,0,0,1), 3939)

IMPLEMENT_MODULE( Fm2uPlugin, m2uPlugin )


bool GetActorByName( const TCHAR* Name, AActor* OutActor, UWorld* InWorld);
void ExecuteCommand(const TCHAR* Str, Fm2uPlugin* Conn);

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

	TcpListener->Stop();
	delete TcpListener;
	TcpListener = NULL;

    // close all clients
	Client->Close();
	Client=NULL;

	delete TickObject;
	TickObject = NULL;
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
    // get data from client
	if(Client==NULL)
		return;

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
			UE_LOG(LogM2U, Log, TEXT("DataNum %i, BytesRead: %i"), Data.Num(), BytesRead);

			// the data we receive is supposed to be ansi, but we will work with TCHAR, so we have to convert
			int32 DestLen = TStringConvert<ANSICHAR,TCHAR>::ConvertedLength((char*)(Data.GetData()),Data.Num());
			//UE_LOG(LogM2U, Log, TEXT("DestLen will be %i"), DestLen);
			TCHAR* Dest = new TCHAR[DestLen+1];
			TStringConvert<ANSICHAR,TCHAR>::Convert(Dest, DestLen, (char*)(Data.GetData()), Data.Num());
			Dest[DestLen]='\0';

			//FString Text(Dest); // FString from tchar array
			//UE_LOG(LogM2U, Log, TEXT("server received %s"), *Text);
			UE_LOG(LogM2U, Log, TEXT("Server received: %s"), Dest);

			ExecuteCommand(Dest, this);

			delete Dest;
		}

		//if(	! Client->Send(Data, Count, BytesSent) )
		//{
		//	UE_LOG(LogM2U, Log, TEXT("server sending answer failed"));
		//}
	}
}

void Fm2uPlugin::SendResponse(const FString& Message)
{
	if( Client == NULL)
		return;

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
		UE_LOG(LogM2U, Log, TEXT("server sending answer failed"));
	}
}

//void HandleReceivedData(FArrayReader& Data)

bool GetActorByName( const TCHAR* Name, AActor** OutActor, UWorld* InWorld = NULL)
{
	if( InWorld == NULL)
	{
		InWorld = GEditor->GetEditorWorldContext().World();
	}
	AActor* Actor;
	//OutActor = FindObject<AActor>( InWorld->GetCurrentLevel(), Name );
	Actor = FindObject<AActor>( ANY_PACKAGE, Name, false );
	// TODO: check if StaticFindObject or StaticFindObjectFastInternal is better
	if( Actor == NULL ) // actor with that name cannot be found
	{
		return false;
	}
	else if( ! Actor->IsValidLowLevelFast() )
	{
		//UE_LOG(LogM2U, Log, TEXT("Actor is NOT valid"));
		return false;
	}
	else
	{
		//UE_LOG(LogM2U, Log, TEXT("Actor is valid"));
		*OutActor=Actor;
		return true;
	}
}

void ExecuteCommand(const TCHAR* Str, Fm2uPlugin* Conn)
{
	if( FParse::Command(&Str, TEXT("Exec")))
	{
		GEditor->Exec(GEditor->GetEditorWorldContext().World(), Str);
	}

	else if( FParse::Command(&Str, TEXT("Test")))
	{
		Conn->SendResponse(TEXT("Hallo"));
	}

	// -- SELECTION --
	else if( FParse::Command(&Str, TEXT("SelectByName")))
	{
		const FString ActorName = FParse::Token(Str,0);
		AActor* Actor = GEditor->SelectNamedActor(*ActorName);
		GEditor->RedrawLevelEditingViewports();
	}
	else if( FParse::Command(&Str, TEXT("DeselectAll")))
	{
		GEditor->SelectNone(true, true, false);
		GEditor->RedrawLevelEditingViewports();
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
	}


	// -- EDITING --
	else if( FParse::Command(&Str, TEXT("TransformObject")))
	{
		const FString ActorName = FParse::Token(Str,0);
		AActor* Actor = NULL;
		UE_LOG(LogM2U, Log, TEXT("Searching for Actor with name %s"), *ActorName);

		if(!GetActorByName(*ActorName, &Actor) || Actor == NULL)
		{
			UE_LOG(LogM2U, Log, TEXT("Actor %s not found or invalid."), *ActorName);
			return;
		}

		const TCHAR* Stream; // used for searching in Str
		//if( Stream != NULL )
		//{	++Stream;  } // skip a space

		// get location
		FVector Loc;
		if( (Stream =  FCString::Strfind(Str,TEXT("T="))) )
		{
			Stream += 3; // skip "T=("
			Stream = GetFVECTORSpaceDelimited( Stream, Loc );
			//UE_LOG(LogM2U, Log, TEXT("Loc %s"), *(Loc.ToString()) );
		}
		else // no translate value in string
		{
			Loc = Actor->GetActorLocation();
		}

		// get rotation
		FRotator Rot;
		if( (Stream =  FCString::Strfind(Str,TEXT("R="))) )
		{
			Stream += 3; // skip "R=("
			Stream = GetFROTATORSpaceDelimited( Stream, Rot, 1.0f );
			//UE_LOG(LogM2U, Log, TEXT("Rot %s"), *(Rot.ToString()) );
		}
		else // no rotate value in string
		{
			Rot = Actor->GetActorRotation();
		}

		// get scale
		FVector Scale;
		if( (Stream =  FCString::Strfind(Str,TEXT("S="))) )
		{
			Stream += 3; // skip "S=("
			Stream = GetFVECTORSpaceDelimited( Stream, Scale );
			//UE_LOG(LogM2U, Log, TEXT("Scc %s"), *(Scale.ToString()) );
			Actor->SetActorScale3D( Scale );
		}

		Actor->TeleportTo( Loc, Rot, false, true);
		Actor->InvalidateLightingCache();
		// Call PostEditMove to update components, etc.
		Actor->PostEditMove( true );
		Actor->CheckDefaultSubobjects();
		// Request saves/refreshes.
		Actor->MarkPackageDirty();

		GEditor->RedrawLevelEditingViewports();
	}
	else if( FParse::Command(&Str, TEXT("DeleteSelected")))
	{
		auto World = GEditor->GetEditorWorldContext().World();
		((UUnrealEdEngine*)GEditor)->edactDeleteSelected(World);
	}
	else if( FParse::Command(&Str, TEXT("RenameObject")))
	{
	}
	else if( FParse::Command(&Str, TEXT("DuplicateObject")))
	{
		const FString ActorName = FParse::Token(Str,0);
		AActor* OrigActor = NULL;
		AActor* Actor = NULL; // the duplicate
		UE_LOG(LogM2U, Log, TEXT("Searching for Actor with name %s"), *ActorName);

		if(!GetActorByName(*ActorName, &OrigActor) || OrigActor == NULL)
		{
			UE_LOG(LogM2U, Log, TEXT("Actor %s not found or invalid."), *ActorName);
			Conn->SendResponse(TEXT("1"));
			return;
		}

		// jump over the next space
		Str = FCString::Strchr(Str,' ');
		if( Str != NULL)
			Str++;

		// the name that is desired for the object
		const FString DupName = FParse::Token(Str,0);

		const FScopedTransaction Transaction( NSLOCTEXT("UnrealEd", "DuplicateActors", "Duplicate Actors") );

		// select only the actor we want to duplicate
		GEditor->SelectNone(true, true, false);
		OrigActor = GEditor->SelectNamedActor(*ActorName); // actor to duplicate
		auto World = GEditor->GetEditorWorldContext().World();
		((UUnrealEdEngine*)GEditor)->edactDuplicateSelected(World->GetCurrentLevel(), false);

		// get the new actor (it will be auto-selected by the editor)
		FSelectionIterator It( GEditor->GetSelectedActorIterator() );
		Actor = static_cast<AActor*>( *It );


		// if there are transform parameters, apply them
		const TCHAR* Stream; // used for searching in Str
		FVector Loc;
		if( (Stream =  FCString::Strfind(Str,TEXT("T="))) )
		{
			Stream += 3; // skip "T=("
			Stream = GetFVECTORSpaceDelimited( Stream, Loc );
			//UE_LOG(LogM2U, Log, TEXT("Loc %s"), *(Loc.ToString()) );
			Actor->SetActorLocation( Loc,false );
		}

		// get rotation
		FRotator Rot;
		if( (Stream =  FCString::Strfind(Str,TEXT("R="))) )
		{
			Stream += 3; // skip "R=("
			Stream = GetFROTATORSpaceDelimited( Stream, Rot, 1.0f );
			//UE_LOG(LogM2U, Log, TEXT("Rot %s"), *(Rot.ToString()) );
			Actor->SetActorRotation( Rot,false );
		}

		// get scale
		FVector Scale;
		if( (Stream =  FCString::Strfind(Str,TEXT("S="))) )
		{
			Stream += 3; // skip "S=("
			Stream = GetFVECTORSpaceDelimited( Stream, Scale );
			//UE_LOG(LogM2U, Log, TEXT("Scc %s"), *(Scale.ToString()) );
			Actor->SetActorScale3D( Scale );
		}


		// Try to set the actor's name to DupName
		// note: a unique name was already assigned during the actual duplicate
		// operation, we could just return that name instead and say "the editor
		// changed the name" but if the DupName can be taken, it will save a lot of
		// extra work on the program side which has to find a new name then.
		GEditor->SetActorLabelUnique( Actor, DupName );
		// get the editor-set name
		const FString AssignedName = Actor->GetFName().ToString();
		// if it is the desired name, everything went fine, if not,
		// send the name as a response to the caller
		if( AssignedName == DupName )
		{
			Conn->SendResponse(TEXT("0"));
		}
		else
		{
			Conn->SendResponse(FString::Printf( TEXT("3 %s"), *AssignedName ) );
		}

		GEditor->RedrawLevelEditingViewports();
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

	}

	// -- OTHER --
	else if( FParse::Command(&Str, TEXT("Undo")))
	{
		GEditor->UndoTransaction();
	}
	else if( FParse::Command(&Str, TEXT("Redo")))
	{
		GEditor->RedoTransaction();
	}
	else if( FParse::Command(&Str, TEXT("GetFreeName")))
	{}

}
