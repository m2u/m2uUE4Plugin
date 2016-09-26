
#pragma once

DECLARE_LOG_CATEGORY_EXTERN(LogM2U, Log, All);

class Fm2uTickObject;

// IP-Address of 0.0.0.0 listens on all local interfaces (all addresses)
//#define DEFAULT_M2U_ENDPOINT FIPv4Endpoint(FIPv4Address(0,0,0,0), 3939)
#define DEFAULT_M2U_ADDRESS FIPv4Address(0,0,0,0)
#define DEFAULT_M2U_PORT 3939

class Fm2uPlugin : public Im2uPlugin, private FSelfRegisteringExec
{
public:
	Fm2uPlugin();

	/* IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static Fm2uPlugin& Get()
	{
		return FModuleManager::LoadModuleChecked< Fm2uPlugin >( "m2uPlugin" );
	}

	/* TcpListener Delegate */
	bool HandleConnectionAccepted( FSocket* ClientSocket, const struct FIPv4Endpoint& ClientEndpoint);

	/* TickObject Delegate */
	void Tick( float DeltaTime );

	/* TCP messaging functions */
	bool GetMessage(FString& Result);
	void SendResponse( const FString& Message);
	void ResetConnection(uint16 Port);

	/* FExec implementation */
	virtual bool Exec( UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar );

protected:
	FSocket* Client;
	class FTcpListener* TcpListener;
	Fm2uTickObject* TickObject;
	class Fm2uOperationManager* OperationManager;

};


