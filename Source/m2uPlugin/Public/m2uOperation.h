
#pragma once

/**
 * Base class for all executable operations of m2u in UE4
 */
class Fm2uOperation
{
protected:

	Fm2uOperationManager* Manager;

public:

	Fm2uOperation( Fm2uOperationManager* Manager = NULL );
	virtual ~Fm2uOperation();

	/**
	 * Try to execute the command. Return false early if not able to execute.
	 */
	virtual bool Execute( FString Cmd, FString& Result ) = 0;
};


/**
 * The Manager holds all instances of available operations and decides which one
 * to use for operating on a certain command.
 * Every Operation has to register one instance in the Manager. The last registered
 * Operation will be the first to be asked to execute a command.
 */
class Fm2uOperationManager
{
protected:

	TArray<Fm2uOperation*> RegisteredOperations;

public:

	~Fm2uOperationManager();

	/**
	 * register an Operation so it may be called when a command needs to be executed */
	void Register( Fm2uOperation* Operation );

	/**
	 * let the first able of the registered Operations handle the Cmd string */
	FString Execute( FString Cmd );
};

// TODO: i want the operations to be able to internally ask for further input
// by sending an (expected) response through the tcp and fetching the answer
// back. This needs to be handled internally because an external object wouldn't
// know to which object to pass a newly received message.
// This functionality is only required for functions that would sho a UI on the
// Program-side and currently there are none of those, that is why that design is
// not implemented yet.
// with direct access to the tcp class, the manager wouldn't need to be called
// with the cmd as string anymore necessarily, but could get the cmd from the tcp
// itself on every tick, though that design change would not be necessary.
