#ifndef _M2UHELPER_H_
#define _M2UHELPER_H_

// disable assignment within conditional expression error in VS
#ifdef _MSC_VER
#pragma warning( disable : 4706 )
#endif

#include "AssetSelection.h"
#include "m2uAssetHelper.h"
#include "Runtime/Launch/Resources/Version.h"

// Functions I'm currently using from this cpp file aren't exported, so they will
// be unresolved symbols on Windows. For now importing the cpp is OK...
// TODO: the required functions should probably be rewritten here instead.
#include "Editor/UnrealEd/Private/ParamParser.cpp"


// Provides functions that are used by most likely more than one command or action
namespace m2uHelper
{

	const FString M2U_GENERATED_NAME(TEXT("m2uGeneratedName"));


/**
   Parse a python-style list from a string to an array containing the contents
   of that list.
   The input string should look like this:
   [name1,name2,name3,name4]
 */
	TArray<FString> ParseList(FString Str)
	{
		FString Chopped = Str.Mid(1,Str.Len()-2); // remove the brackets
		TArray<FString> Result;
		Chopped.ParseIntoArray( Result, TEXT(","), false);
		return Result;
	}

/**
 *  Try to find an Actor by name.
 *
 *  @param Name The name to look for.
 *  @param OutActor This will be the found Actor or `nullptr`.
 *  @param InWorld The world in which to search for the Actor. If null,
 *     get the current world from the Editor.
 *
 *  @return true if found and valid, false otherwise.
 */
bool GetActorByName(const TCHAR* Name, AActor** OutActor, UWorld* InWorld=nullptr)
{
	if (InWorld == nullptr)
	{
		InWorld = GEditor->GetEditorWorldContext().World();
	}

	ULevel* CurrentLevel = InWorld->GetCurrentLevel();
	FName ObjectFName = FName(Name);
	UObject* Object;
	Object = StaticFindObjectFast(AActor::StaticClass(), CurrentLevel, ObjectFName,
	                              /*ExactClass=*/false, /*AnyPackage=*/false);
	AActor* Actor = Cast<AActor>(Object);
	if (Actor == nullptr)
	{
		// Actor with that name cannot be found.
		return false;
	}
	else if (! Actor->IsValidLowLevel())
	{
		//UE_LOG(LogM2U, Log, TEXT("Actor is NOT valid"));
		return false;
	}
	else
	{
		*OutActor = Actor;
		return true;
	}
}


	/**
	 * Create a string that is valid to be used as an object name by
	 * removing all 'invalid' characters from it.
	 */
	FString MakeValidNameString(const FString& Name)
	{
		FString GeneratedName = Name;
		for (int32 BadCharacterIndex = 0; BadCharacterIndex < ARRAY_COUNT(
				 INVALID_OBJECTNAME_CHARACTERS) - 1; ++BadCharacterIndex)
		{
			const TCHAR TestChar[2] = {
				INVALID_OBJECTNAME_CHARACTERS[BadCharacterIndex], 0 };
			GeneratedName.ReplaceInline(TestChar, TEXT(""));
		}
		return GeneratedName;
	}


	/**
	 * Find a free (unused) name based on the Name string provided.
	 *
	 * This will be achieved by increasing or adding a number-suffix
	 * until the name is unique.
	 *
	 * @param Name A name string with or without a number suffix on
	 *     which to build onto.
	 */
	FName GetFreeName(const FString& Name)
	{
		FString GeneratedName = MakeValidNameString(Name);
		// If there is no valid name left, use a default base name.
		FName TestName(*GeneratedName);
		if (TestName == NAME_None)
		{
			TestName = FName(*M2U_GENERATED_NAME);
		}

		// Check only inside the current level, it is unlikely that we
		// would be interested in names anywhere else.
		UWorld* CurrentWorld = GEditor->GetEditorWorldContext().World();
		ULevel* CurrentLevel = CurrentWorld->GetCurrentLevel();
		UObject* ExistingObject;

		// Increase the suffix until there is no ExistingObject found.
		for(;;)
		{
			ExistingObject = StaticFindObjectFast(AActor::StaticClass(),
			                                      CurrentLevel, TestName,
			                                      /*ExactClass=*/false,
			                                      /*AnyPackage=*/false);
			if (! ExistingObject)
			{
				// Current name is not in use.
				break;
			}
			TestName.SetNumber(TestName.GetNumber() + 1);
		}
		return TestName;

	} // FName GetFreeName()


/**
 * void SetActorTransformRelativeFromText(AActor* Actor, const TCHAR* Stream)
 *
 * Set the Actors relative transformations to the values provided in text-form
 * T=(x y z) R=(x y z) S=(x y z)
 * If one or more of T, R or S is not present in the String, they will be ignored.
 *
 * Relative transformations are the actual transformation values you see in the 
 * Editor. They are equivalent to object-space transforms in maya for example.
 *
 * Setting world-space transforms using SetActorLocation or so will yield fucked
 * up results when using nested transforms (parenting actors).
 *
 * The Actor has to be valid, so check before calling this function!
 */
	void SetActorTransformRelativeFromText(AActor* Actor, const TCHAR* Str)
	{
		const TCHAR* Stream; // used for searching in Str
		//if( Stream != NULL )
		//{	++Stream;  } // skip a space

		// get location
		FVector Loc;
		if( (Stream =  FCString::Strfind(Str,TEXT("T="))) )
		{
			Stream += 3; // skip "T=("
			Stream = GetFVECTORSpaceDelimited( Stream, Loc );
			//UE_LOG(LogM2U, Log, TEXT("Loc %s"), *(Loc.ToString()) );
			Actor->SetActorRelativeLocation( Loc, false );
		}

		// get rotation
		FRotator Rot;
		if( (Stream =  FCString::Strfind(Str,TEXT("R="))) )
		{
			Stream += 3; // skip "R=("
			Stream = GetFROTATORSpaceDelimited( Stream, Rot, 1.0f );
			//UE_LOG(LogM2U, Log, TEXT("Rot %s"), *(Rot.ToString()) );
			Actor->SetActorRelativeRotation( Rot, false );
		}

		// get scale
		FVector Scale;
		if( (Stream =  FCString::Strfind(Str,TEXT("S="))) )
		{
			Stream += 3; // skip "S=("
			Stream = GetFVECTORSpaceDelimited( Stream, Scale );
			//UE_LOG(LogM2U, Log, TEXT("Scc %s"), *(Scale.ToString()) );
			Actor->SetActorRelativeScale3D( Scale );
		}

		Actor->InvalidateLightingCache();
		// Call PostEditMove to update components, etc.
		Actor->PostEditMove( true );
		Actor->CheckDefaultSubobjects();
		// Request saves/refreshes.
		Actor->MarkPackageDirty();

	}// void SetActorTransformRelativeFromText()




} // namespace m2uHelper
#endif /* _M2UHELPER_H_ */
