#include "m2uPluginPrivatePCH.h"

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


Fm2uPlugin::Fm2uPlugin()
	:Client(NULL),
	 TcpListener(NULL)
{
}


void Fm2uPlugin::StartupModule()
{
	UE_LOG(LogM2U, Log, TEXT("Starting up m2u module."));
	if (!GIsEditor)
	{
		UE_LOG(LogM2U, Log, TEXT("Not in Editor, canceling."));
		return;
	}

	this->ResetConnection( DEFAULT_M2U_PORT );

	this->TickObject = new Fm2uTickObject(this);

	this->OperationManager = new Fm2uOperationManager();
	CreateBuiltinOperations(OperationManager);

	m2uUI::RegisterUI();

	// Disable the CPU throttling feature. Otherwise the Editor won't
	// react to messages sent by m2u clients.
	UEditorPerProjectUserSettings* Settings = GetMutableDefault<UEditorPerProjectUserSettings>();
	Settings->bThrottleCPUWhenNotForeground = false;
}


void Fm2uPlugin::ShutdownModule()
{
    // Close all clients.
	if (this->Client != nullptr)
	{
		this->Client->Close();
		this->Client = nullptr;
	}

	this->TcpListener->Stop();
	delete this->TcpListener;
	this->TcpListener = nullptr;

	delete this->TickObject;
	this->TickObject = nullptr;

	delete this->OperationManager;
	this->OperationManager = nullptr;

	m2uUI::UnregisterUI();
}


bool Fm2uPlugin::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	if (FParse::Command(&Cmd, TEXT("m2uCloseConnection")))
	{
		uint16 Port = DEFAULT_M2U_PORT;
		FString PortString;
		if (FParse::Token(Cmd, PortString, 0))
		{
			Port = FCString::Atoi(*PortString);
		}
		this->ResetConnection(Port);
		return true;
	}
	else if (FParse::Command(&Cmd, TEXT("m2uBatchFileParse")))
	{
		FString Filename;
		if (FParse::Token(Cmd, Filename, 0))
		{
			m2uBatchFileParse(Filename);
		}
		return true;
	}
	else if (FParse::Command(&Cmd, TEXT("m2uDo")))
	{
		// Execute an Action without using tcp connection
		this->OperationManager->Execute(FString(Cmd));
		return true;
	}
	return false;
}


void Fm2uPlugin::ResetConnection(uint16 Port)
{
	if (this->Client != nullptr)
	{
		this->Client->Close();
		this->Client = nullptr;
	}
	if (this->TcpListener != nullptr)
	{
		this->TcpListener->Stop();
		delete this->TcpListener;
	}
	UE_LOG(LogM2U, Log, TEXT("Hosting on Port %i"), Port);
	this->TcpListener = new FTcpListener(
		FIPv4Endpoint(DEFAULT_M2U_ADDRESS, Port));
	this->TcpListener->OnConnectionAccepted().BindRaw(
		this, &Fm2uPlugin::HandleConnectionAccepted);
}


bool Fm2uPlugin::HandleConnectionAccepted(FSocket* ClientSocket,
                                          const FIPv4Endpoint& ClientEndpoint)
{
	if (this->Client == nullptr)
	{
		this->Client = ClientSocket;
		int32 NewSize;
		this->Client->SetReceiveBufferSize(4000000, NewSize); // TODO: set normal size
		UE_LOG(LogM2U, Log, TEXT("Connected on Port %i, Buffersize %i."),
		       this->Client->GetPortNo(), NewSize);
		return true;
	}
	UE_LOG(LogM2U, Log, TEXT("Connection declined"));
	return false;
}


void Fm2uPlugin::Tick(float DeltaTime)
{
	// Valid and connected?
	if (this->Client != nullptr &&
	    this->Client->GetConnectionState() == SCS_Connected)
	{
		// Get the message, do stuff, and tell the caller what happened.
		FString Message;
		if (this->GetMessage(Message))
		{
			// TODO: Add batch-parse-message and execute multiple, newline-
			//   divided operations in one go
			FString Result = this->OperationManager->Execute(Message);
			this->SendResponse(Result);
		}
	}
}


/**
 * Get all data from client and create one long FString from it.
 * Return the result when the client is out of pending data.
 */
bool Fm2uPlugin::GetMessage(FString& Result)
{
	uint32 DataSize = 0;
	while (this->Client->HasPendingData(DataSize) && DataSize > 0)
	{
		//UE_LOG(LogM2U, Log, TEXT("pending data size %i"), DataSize);
        // create data array to read from client
		FArrayReader Data;
		Data.SetNumUninitialized(DataSize);

		int32 BytesRead = 0;
        // Read pending data into the Data array reader.
		if (this->Client->Recv(Data.GetData(), Data.Num(), BytesRead))
		{
			//UE_LOG(LogM2U, Log, TEXT("DataNum %i, BytesRead: %i"), Data.Num(), BytesRead);

			// The data we receive is supposed to be ansi, but we will
			// work with TCHAR, so we have to convert.
			int32 DestLen = TStringConvert<ANSICHAR,TCHAR>::ConvertedLength(
				(char*)(Data.GetData()), Data.Num());
			//UE_LOG(LogM2U, Log, TEXT("DestLen will be %i"), DestLen);
			TCHAR* Dest = new TCHAR[DestLen+1];
			TStringConvert<ANSICHAR,TCHAR>::Convert(
				Dest, DestLen, (char*)(Data.GetData()), Data.Num());
			Dest[DestLen]='\0';

			//FString Text(Dest); // FString from tchar array
			//UE_LOG(LogM2U, Log, TEXT("server received %s"), *Text);
			//UE_LOG(LogM2U, Log, TEXT("Server received: %s"), Dest);

			Result += Dest;

			delete Dest;
		}
	} // while
	if (!Result.IsEmpty())
		return true;
	else
		return false;
}


void Fm2uPlugin::SendResponse(const FString& Message)
{
	if (this->Client != nullptr &&
	    this->Client->GetConnectionState() == SCS_Connected)
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
