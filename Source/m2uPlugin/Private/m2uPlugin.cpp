#include "m2uPlugin.h"

#include "ActorEditorUtils.h"
#include "UnrealEd.h"
#include "Core.h"
#include "Runtime/Sockets/Private/BSDSockets/SocketSubsystemBSD.h"
#include "Networking.h"
#include "Editor/EditorPerformanceSettings.h"

#include "m2uTickObject.h"
#include "m2uHelper.h"
#include "m2uBatchFileParse.h"
#include "m2uBuiltinOperations.h"
#include "m2uUI.h"



DEFINE_LOG_CATEGORY( LogM2U )

IMPLEMENT_MODULE( Fm2uPlugin, m2uPlugin )


const float READ_BODY_TIMEOUT_S = 3.f;


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
	UEditorPerformanceSettings* Settings = GetMutableDefault<UEditorPerformanceSettings>();
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
	// If we have an old, maybe still used connection, close it first.
	if (this->Client &&
		this->Client->GetConnectionState() == SCS_Connected)
	{
		this->Client->Close();
	}
	// Note: We accept new connections, even though the old one may
	// still be in use. The problem is, that a proper close from the
	// client side seems not to set the socket state to not-connected,
	// so we don't know if the old client disconnected or not.

	this->Client = ClientSocket;
	int32 NewSize;
	this->Client->SetReceiveBufferSize(4096, NewSize);
	UE_LOG(LogM2U, Log, TEXT("Connected on Port %i, Buffersize %i."),
		   this->Client->GetPortNo(), NewSize);
	return true;
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
 * Check if there is a message from the client. When there is a
 * message, read it completely and write it into the `Result`
 * argument.
 *
 * If a message has been received, the return value will be true,
 * false otherwise.
 */
bool Fm2uPlugin::GetMessage(FString& Result)
{
	FArrayReader Data;  // Array for reading bytes from input stream.
	uint32 PendingDataSize = 0;
	int32 BytesRead = 0;

	if (! this->Client->HasPendingData(PendingDataSize)) {
		// No message in stream.
		return false;
	}

	// Read the message header, which must be a uint32 that represents
	// the number of bytes of the actual message.

	if (PendingDataSize < 4) {
		// We don't support reading the ContentLength header in
		// chunks.  This should be very unlikely to happen, unless the
		// data is not actually a header.
		UE_LOG(LogM2U, Error, TEXT("Pending data does not contain a complete header. "
								   "Flushing input stream."));
		// Clear the input stream for the next message.
		Data.SetNumUninitialized(PendingDataSize);
		this->Client->Recv(Data.GetData(), Data.Num(), BytesRead);
		return false;
	}

	uint32 ContentLength = 0;
	this->Client->Recv((uint8*)&ContentLength, 4, BytesRead);
	if (BytesRead != 4) {
		// Not sure if this might actually happen. Recv should read
		// exactly the 4 bytes we tell it to.
		UE_LOG(LogM2U, Error, TEXT("Did not read complete header."));
		return false;
	}

	// ContentLength is sent in Big Endian (aka Network Endian), so
	// depending on the platform, this needs to be converted.
	ContentLength = ntohl(ContentLength);
	UE_LOG(LogM2U, Log, TEXT("Received ContentLength: %i"), ContentLength);

	// Read the message body. Read only as many bytes as indicated by
	// the header. If there is not enough data on the stream, try
	// again until the data has been written or we time out.
	uint32 TimeReadBodyStart = FPlatformTime::Cycles();
	uint32 TimeReadBodyDuration = 0;
	uint32 TotalBodyBytesRead = 0;
	while (TotalBodyBytesRead < ContentLength)
	{
		if (this->Client->HasPendingData(PendingDataSize) && PendingDataSize > 0)
		{
			// If there is more data in the stream than is anticipated,
			// make sure we don't read over the message end.
			if (TotalBodyBytesRead + PendingDataSize > ContentLength) {
				PendingDataSize = ContentLength - TotalBodyBytesRead;
			}

			// Transfer the data from the stream into the array reader.
			Data.SetNumUninitialized(PendingDataSize);
			this->Client->Recv(Data.GetData(), Data.Num(), BytesRead);
			TotalBodyBytesRead += BytesRead;

			// The data we received should be ANSI, but we will work
			// with TCHAR internally, so we have to convert.
			int32 DestLen = TStringConvert<ANSICHAR,TCHAR>::ConvertedLength(
			    (char*)(Data.GetData()), Data.Num());
			TCHAR* Dest = new TCHAR[DestLen+1];
			Dest[DestLen] = '\0';
			// Note: It might be safer to first read the whole message
			// and then convert to TCHAR.  But we don't handle any
			// encoding, so this unlikely matters at the moment.
			TStringConvert<ANSICHAR,TCHAR>::Convert(
			    Dest, DestLen, (char*)(Data.GetData()), Data.Num());

			Result += Dest;
			delete Dest;
		}
		else
		{
			// If we are waiting for data for quite some time, that
			// data may never come. Either the header was wrong, or
			// some other mistake was made. But we have to make sure
			// we don't hang in this loop forever.
			TimeReadBodyDuration = FPlatformTime::ToMilliseconds(
			    FPlatformTime::Cycles() - TimeReadBodyStart) / 1000.f;
			if (TimeReadBodyDuration > READ_BODY_TIMEOUT_S) {
				UE_LOG(LogM2U, Error, TEXT("Timeout while reding message body. "
										   "ContentLength too big?"));
				return false;
			}
		}
	}

	if (!Result.IsEmpty())
		return true;
	else
		return false;
}


/**
 * Send a message back to the client.
 *
 * If the client is not connected or an error occurs while sending
 * the message, the return value will be false.
 *
 * A message should only be sent to the client, when it is expected.
 * Otherwise the message may not be read from the clients input stream,
 * causing problems with future messages.
 */
bool Fm2uPlugin::SendResponse(const FString& Message)
{
	if (this->Client == nullptr ||
	    this->Client->GetConnectionState() != SCS_Connected)
	{
		return false;
	}

	int32 DestLen = TStringConvert<TCHAR,ANSICHAR>::ConvertedLength(*Message, Message.Len());
	uint8* Dest = new uint8[DestLen];
	TStringConvert<TCHAR,ANSICHAR>::Convert((ANSICHAR*)Dest, DestLen, *Message, Message.Len());

	// ContentLength must be sent in Big Endian (aka Network
	// Endian), so depending on the platform, this needs to be
	// converted.
	int32 _ContentLength = htonl(DestLen);
	int32 BytesSent = 0;
	if (! Client->Send((uint8*)&_ContentLength, 4, BytesSent)){
		UE_LOG(LogM2U, Error, TEXT("TCP Server sending answer header failed."));
		return false;
	}

	int32 TotalBytesSent = 0;
	while (TotalBytesSent < DestLen)
	{
		if (! Client->Send(Dest + TotalBytesSent, DestLen - TotalBytesSent, BytesSent))
		{
			UE_LOG(LogM2U, Error, TEXT("TCP Server sending answer body failed."));
			return false;
		}
		TotalBytesSent += BytesSent;
	}
	return true;
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
