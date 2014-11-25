
#pragma once

DECLARE_LOG_CATEGORY_EXTERN(LogM2U, Log, All);

class Fm2uTickObject;

class Fm2uPlugin : public Im2uPlugin, private FSelfRegisteringExec
{
public:
	Fm2uPlugin();

	/* IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/* TcpListener Delegate */
	bool HandleConnectionAccepted( FSocket* ClientSocket, const class FIPv4Endpoint& ClientEndpoint);

	/* TickObject Delegate */
	void Tick( float DeltaTime );

	/* TCP messaging functions */
	bool GetMessage(FString& Result);
	void SendResponse( const FString& Message);

	/* FExec implementation */
	virtual bool Exec( UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar );

protected:
	FSocket* Client;
	class FTcpListener* TcpListener;
	Fm2uTickObject* TickObject;
	class Fm2uOperationManager* OperationManager;

};


