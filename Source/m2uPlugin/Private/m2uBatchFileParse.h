#ifndef _M2UBATCHFILEPARSE_H_
#define _M2UBATCHFILEPARSE_H_


/**
   this is a temporary solution to parse a file for commands that the m2u actions 
   can execute. This will be integrated into m2u action classes once I find time
   to refactor this all.
   This function expects the import operations to be listed before the add operations
   of course it will not perform intelligent checks if that is the case!
 */
bool m2uBatchFileParse(FString& Filename)
{
	FString Result;
	if( FFileHelper::LoadFileToString(Result, *Filename) )
	{
		UE_LOG(LogM2U, Error, TEXT("Batch File parsing is currently not implemented"));
		return false;
		UE_LOG(LogM2U, Error, TEXT("Parsing File \"%s\" for commands."), *Filename);

		FString ImpCmd = TEXT("ImportAssetsBatch");
		FString AddCmd = TEXT("AddActorBatch");

		uint32 ImpStart = Result.Find(ImpCmd) + ImpCmd.Len();
		uint32 AddStart = Result.Find(AddCmd);
		uint32 ImpLength = AddStart - ImpStart;

		FString ImpBlock = Result.Mid(ImpStart, ImpLength);
		FString AddBlock = Result.RightChop(AddStart + AddCmd.Len());

		//m2uActions::ImportAssetsBatch(*ImpBlock);
		//m2uActions::AddActorBatch(*AddBlock);
		
		return true;
	}
	UE_LOG(LogM2U, Error, TEXT("File \"%s\" not found."), *Filename);
	return false;
}



#endif /* _M2UBATCHFILEPARSE_H_ */
