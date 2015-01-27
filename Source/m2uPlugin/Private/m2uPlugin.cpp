// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "m2uPluginPrivatePCH.h"
//#include "m2uPlugin.generated.inl"

#include "Networking.h"
#include "ActorEditorUtils.h"
#include "UnrealEd.h"

#include "Runtime/Engine/Classes/Engine/TextRenderActor.h"

#include "m2uHelper.h"
#include "m2uBatchFileParse.h"

#include "m2uBuiltinOperations.h"

#include "m2uUI.h"

DEFINE_LOG_CATEGORY( LogM2U )


IMPLEMENT_MODULE( Fm2uPlugin, m2uPlugin )


//bool GetActorByName( const TCHAR* Name, AActor* OutActor, UWorld* InWorld);
FString ExecuteCommand(const TCHAR* Str/*, Fm2uPlugin* Conn*/);

Fm2uPlugin::Fm2uPlugin()
	:Client(NULL),
	 TcpListener(NULL)
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

	ResetConnection( DEFAULT_M2U_PORT );

	TickObject = new Fm2uTickObject(this);
	
	OperationManager = new Fm2uOperationManager();
	CreateBuiltinOperations(OperationManager);

	m2uUI::RegisterUI();
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

	delete OperationManager;
	OperationManager = NULL;

	m2uUI::UnregisterUI();
}

bool Fm2uPlugin::Exec( UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar )
{
	if( FParse::Command(&Cmd, TEXT("m2uCloseConnection")) )
	{
		uint16 Port = DEFAULT_M2U_PORT;
		FString PortString;
		if( FParse::Token(Cmd, PortString, 0))
		{
			Port = FCString::Atoi(*PortString);
		}
		ResetConnection(Port);
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
		//ExecuteCommand(Cmd);
		OperationManager -> Execute(FString(Cmd));
		return true;
	}
	return false;
}

void Fm2uPlugin::ResetConnection(uint16 Port)
{
	if(Client != NULL)
	{
		Client->Close();
		Client=NULL;
	}
	if(TcpListener != NULL)
	{
		TcpListener->Stop();
		delete TcpListener;
	}
	UE_LOG(LogM2U, Log, TEXT("Hosting on Port %i"), Port);
	TcpListener = new FTcpListener( FIPv4Endpoint(DEFAULT_M2U_ADDRESS, Port) );
	TcpListener->OnConnectionAccepted().BindRaw(this, &Fm2uPlugin::HandleConnectionAccepted);
}

bool Fm2uPlugin::HandleConnectionAccepted( FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint)
{
	if(Client==NULL)
	{
		Client = ClientSocket;
		UE_LOG(LogM2U, Log, TEXT("Connected on Port %i"),Client->GetPortNo());
		return true;
	}
	UE_LOG(LogM2U, Log, TEXT("Connection declined"));
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
			//FString Result = ExecuteCommand(*Message);
			// TODO: add batch-parse-message and execute multiple, newline-divided
			// operations in one go
			FString Result = OperationManager->Execute(Message);
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


// TODO: remove
/*FString ExecuteCommand(const TCHAR* Str)
{
	if( FParse::Command(&Str, TEXT("TestActor")))
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
*/
