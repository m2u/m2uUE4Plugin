
#include "m2uPluginPrivatePCH.h"


Um2uFbxFactory::Um2uFbxFactory(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
}



/**
   This is called sometime before the Factory is used... It was once intended to 
   show a UI to ask for user input.
   This allows us to set stuff so the parent-class code won't show the UI, or even
   set values that we read from some input.
 */
bool Um2uFbxFactory::ConfigureProperties()
{
	// auto decide if StaticM or SkelM
	bDetectImportTypeOnImport = true;
	// do not show the UI
	bShowOption = false;

	return true;
}
