#pragma once

#include "m2uOperation.h"


/**
 * Tests for message interpretation and communication.
 *
 * A client-side test should call these commands and make sure the
 * return value is correct.
 */
class Fm2uOpTests : public Fm2uOperation
{
public:

	Fm2uOpTests( Fm2uOperationManager* Manager = NULL )
		:Fm2uOperation( Manager ){}

	bool Execute( FString Cmd, FString& Result ) override
	{
		const TCHAR* Str = *Cmd;

		// Test that a message is read completely. Return the length
		// of the message as result text.  This should match with the
		// length on the client's side.
		if (FParse::Command(&Str, TEXT("TestMessageSize")))
		{
			Result = FString::Printf(TEXT("%i"), Cmd.Len());
			return true;
		}


		// Initialize a clean scene with specific test data.
		if (FParse::Command(&Str, TEXT("TestInitializeScene")))
		{
			UClass& AssetClass = *UActorFactoryBasicShape::StaticClass();
			UActorFactory* ActorFactory = GEditor->FindActorFactoryByClass(&AssetClass);
			UObject* CubeAsset = LoadObject<UStaticMesh>(nullptr,*UActorFactoryBasicShape::BasicCube.ToString());
			UObject* SphereAsset = LoadObject<UStaticMesh>(nullptr,*UActorFactoryBasicShape::BasicSphere.ToString());

			// GEditor->CreateNewMapForEditing();
			GUnrealEd->NewMap();
			ULevel* Level = GEditor->GetEditorWorldContext().World()->GetCurrentLevel();
			AActor* Cube1 = ActorFactory->CreateActor(CubeAsset, Level, FTransform::Identity, RF_Transactional, FName(TEXT("cube_1")));
			AActor* Cube2 = ActorFactory->CreateActor(CubeAsset, Level, FTransform::Identity, RF_Transactional, FName(TEXT("cube_2")));
			Cube2->Rename(TEXT("cube_2"), nullptr, REN_DontCreateRedirectors);
			AActor* Cube3 = ActorFactory->CreateActor(CubeAsset, Level, FTransform::Identity, RF_Transactional, FName(TEXT("cube_3")));
			Cube3->Rename(TEXT("cube_3"), nullptr, REN_DontCreateRedirectors);
			AActor* Sphere1 = ActorFactory->CreateActor(SphereAsset, Level, FTransform::Identity, RF_Transactional, FName(TEXT("sphere_1")));

			Result = FString(TEXT("Initialized"));
			return true;
		}

		return false;
	}
};
