// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "m2uPluginPrivatePCH.h"
#include "Networking.h"

DEFINE_LOG_CATEGORY( LogM2U )

#define DEFAULT_M2U_ENDPOINT FIPv4Endpoint(FIPv4Address(127,0,0,1), 3939)

IMPLEMENT_MODULE( Fm2uPlugin, m2uPlugin )

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
		UE_LOG(LogM2U, Log, TEXT("pending data size %i"), DataSize);
// create data array to read from client
		//FArrayReaderPtr Data = MakeShareable(new FArrayReader(true));
		FArrayReader Data;
		Data.Init(DataSize);

		int32 BytesRead = 0;
// read the data
		if (Client->Recv(Data.GetData(), Data.Num(), BytesRead) )
		{
			UE_LOG(LogM2U, Log, TEXT("DataNum %i, BytesRead: %i"), Data.Num(), BytesRead);
			//uint8* traw = new uint8[Data.Num()];
			//*Data << traw; // retrieve received data from the archive
			int32 DestLen = TStringConvert<ANSICHAR,TCHAR>::ConvertedLength((char*)(Data.GetData()),Data.Num());
			TCHAR* Dest = new TCHAR[DestLen];
			TStringConvert<ANSICHAR,TCHAR>::Convert(Dest, DestLen, (char*)(Data.GetData()), Data.Num());
			//FUTF8ToTCHAR_Convert::Convert(Dest, Data.Num(), (ANSICHAR*)(Data.GetData()), Data.Num());
			//FString Text(ANSI_TO_TCHAR(Data.GetData()));
			FString Text(Dest);
			//Text.FromBlob(Dest,Data.Num());
			UE_LOG(LogM2U, Log, TEXT("server received %s"), *Text);
			//delete traw;
			delete Dest;
		}
		
		//if(	! Client->Send(Data, Count, BytesSent) )
		//{
		//	UE_LOG(LogM2U, Log, TEXT("server sending answer failed"));
		//}
	}
}
