#pragma once
// Asset Import and Export Operations

#include "UnrealEd.h"
#include "ActorEditorUtils.h"

#include "m2uOperation.h"
#include "m2uHelper.h"
#include "m2uAssetHelper.h"


class Fm2uOpAssetExport : public Fm2uOperation
{
public:

	Fm2uOpAssetExport( Fm2uOperationManager* Manager = NULL )
		:Fm2uOperation( Manager ){}

	bool Execute( FString Cmd, FString& Result ) override
	{
		const TCHAR* Str = *Cmd;
		bool DidExecute = false;

		if (FParse::Command(&Str, TEXT("ExportAsset")))
		{
			UE_LOG(LogM2U, Log, TEXT("Received ExportAsset: %s"), Str);
			FString AssetName = FParse::Token(Str, 0);
			FString ExportPath = FParse::Token(Str, 0);
			m2uAssetHelper::ExportAsset(AssetName, ExportPath);
			DidExecute = true;
		}

		Result = TEXT("Ok");
		return DidExecute;
	}
};


class Fm2uOpAssetImport : public Fm2uOperation
{
public:

	Fm2uOpAssetImport(Fm2uOperationManager* Manager=nullptr)
		:Fm2uOperation(Manager){}

	bool Execute(FString Cmd, FString& Result) override
	{
		const TCHAR* Str = *Cmd;
		bool DidExecute = false;

		if (FParse::Command(&Str, TEXT("ImportAssets")))
		{
			Result = ImportAssets(Str);
			DidExecute = true;
		}
		else if (FParse::Command(&Str, TEXT("ImportAssetsBatch")))
		{
			Result = ImportAssetsBatch(Str);
			DidExecute = true;
		}

		return DidExecute;
	}

	/**
	 * Import all assets listed after the destination path into the
	 * destination path.
	 *
	 * If paths to files are specified, all files will be imported
	 * directly into the destination.  If path end in folders instead
	 * of a files, all files in that folder and its subfolders will be
	 * imported, recreating the folder structure with the specified
	 * folder being at the same level as the destination path.
	 */
	FString ImportAssets(const TCHAR* Str)
	{
		bool bForceNoOverwrite = false;
		if (FParse::Bool(Str, TEXT("ForceNoOverwrite="), bForceNoOverwrite))
		{
			// jump over the next space
			Str = FCString::Strchr(Str,' ');
			if( Str != NULL)
				Str++;
		}
		FString RootDestinationPath = FParse::Token(Str,0);
		TArray<FString> Files;
		FString AssetFile;
		while( FParse::Token(Str, AssetFile, 0) )
		{
			Files.Add(AssetFile);
		}
		m2uAssetHelper::ImportAssets(Files, RootDestinationPath, false, bForceNoOverwrite/*, &GetUserInput*/ );
		return TEXT("Ok");
	}

	/**
	 * Import all assets listed in the string to its assocated
	 * destination so the string must always contain
	 * "/DestinationPath" "/FilePath" separated by whitespaces for
	 * each file to import.  The DestinationPath must not contain the
	 * AssetName!  Use this to mirror a folder structure by explicitly
	 * telling the Editor to create the destination path for each
	 * source file.
	 *
	 * Of course if one of the specified AssetSource values is a
	 * Folder, all files and subfolders will be imported.
	 */
	FString ImportAssetsBatch(const TCHAR* Str)
	{
		FString AssetDestination;
		FString AssetSource;
		bool bForceNoOverwrite = false;
		if(FParse::Bool(Str, TEXT("ForceNoOverwrite="), bForceNoOverwrite))
		{
			// jump over the next space
			Str = FCString::Strchr(Str,' ');
			if( Str != NULL)
				Str++;
		}
		while( FParse::Token(Str, AssetDestination, 0) )
		{
			if( FParse::Token(Str, AssetSource, 0) )
			{
				TArray<FString> Files;
				Files.Add(AssetSource);
				m2uAssetHelper::ImportAssets(Files, AssetDestination, false, bForceNoOverwrite/*, &GetUserInput*/);
			}
			else // there is an uneven list of Destination<->FilePath infos
			{
				// TODO: maybe store all associations in a pair-list first,
				// and start the import process only if they are even. Because
				// the reason for unevenness may be earlier than the end of the list.
				// We would import crap then maybe or create invalid destination paths
				// which in turn might crash the editor.
				UE_LOG(LogM2U, Error, TEXT("Uneven list of Destination<->FilePath infos for Import."));
				return TEXT("1");
			}
		}
		return TEXT("Ok");
	}

};
