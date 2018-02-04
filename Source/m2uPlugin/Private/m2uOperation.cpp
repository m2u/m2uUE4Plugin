#include "m2uOperation.h"


Fm2uOperation::Fm2uOperation(Fm2uOperationManager* Manager)
{
	this->Manager = Manager;
	if (Manager != nullptr)
	{
		Manager->Register(this);
	}
}


Fm2uOperation::~Fm2uOperation()
{}


Fm2uOperationManager::~Fm2uOperationManager()
{
	for (Fm2uOperation* Op : this->RegisteredOperations)
	{
		delete Op;
	}
}


void Fm2uOperationManager::Register(Fm2uOperation* Operation)
{
	this->RegisteredOperations.Insert(Operation, 0);
}


FString Fm2uOperationManager::Execute(FString Cmd)
{
	for (Fm2uOperation* Operation : this->RegisteredOperations )
	{
		FString Result;
		if (Operation->Execute(Cmd, Result))
		{
			return Result;
		}
	}
	// No Operation is registered which could handle that command.
	UE_LOG(LogM2U, Warning, TEXT("Command not found: %s"), *Cmd);
	return TEXT("Command Not Found");
}
