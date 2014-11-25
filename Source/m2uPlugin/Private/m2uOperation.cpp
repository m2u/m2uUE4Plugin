
#include "m2uPluginPrivatePCH.h"
#include "m2uOperation.h"

Fm2uOperation::Fm2uOperation( Fm2uOperationManager* Manager )
{
	this->Manager = Manager;
	if( Manager != NULL )
	{
		Manager->Register(this);
	}
}

Fm2uOperation::~Fm2uOperation()
{}


Fm2uOperationManager::~Fm2uOperationManager()
{
	for( Fm2uOperation* Op : RegisteredOperations )
		delete Op;
}

void Fm2uOperationManager::Register( Fm2uOperation* Operation )
{
	RegisteredOperations.Insert(Operation,0);
}

FString Fm2uOperationManager::Execute( FString Cmd )
{
	for( Fm2uOperation* Operation : RegisteredOperations )
	{
		FString Result;
		if( Operation -> Execute(Cmd, Result) )
		{
			return Result;
		}
	}
	// no Operation could handle that command
	UE_LOG(LogM2U, Warning, TEXT("Command not found: %s"), *Cmd);
	return TEXT("Command Not Found");
}
