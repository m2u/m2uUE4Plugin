
#pragma once

DECLARE_LOG_CATEGORY_EXTERN(LogM2U, Log, All);

class Fm2uTickObject;

class Fm2uPlugin : public Im2uPlugin, private FSelfRegisteringExec
{
public:
	Fm2uPlugin();

	/* IModuleInterface implementation */
	virtual void StartupModule() OVERRIDE;
	virtual void ShutdownModule() OVERRIDE;
	bool HandleConnectionAccepted( FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint);
	void Tick( float DeltaTime );
	void SendResponse( const FString& Message);

	/* FExec implementation */
	virtual bool Exec( UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar );

protected:
	FSocket* Client;
	class FTcpListener* TcpListener;
	Fm2uTickObject* TickObject;

};


