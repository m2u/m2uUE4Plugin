#pragma once
// create all known operations for the operation manager

#include "m2uOperation.h"

#include "m2uOpHelloWorld.h"

#include "m2uOpAsset.h"
#include "m2uOpCamera.h"
#include "m2uOpExec.h"
#include "m2uOpFetch.h"
#include "m2uOpLayer.h"
#include "m2uOpObject.h"
#include "m2uOpSelection.h"
#include "m2uOpTransaction.h"
#include "m2uOpVisibility.h"

void CreateBuiltinOperations( Fm2uOperationManager* Manager )
{
	new Fm2uOpHelloWorld(Manager);

	new Fm2uOpAssetExport(Manager);
	new Fm2uOpAssetImport(Manager);

	new Fm2uOpCamera(Manager);

	new Fm2uOpExec(Manager);

	new Fm2uOpLayer(Manager);

	new Fm2uOpObjectTransform(Manager);
	new Fm2uOpObjectName(Manager);
	new Fm2uOpObjectDelete(Manager);
	new Fm2uOpObjectDuplicate(Manager);
	new Fm2uOpObjectAdd(Manager);
	new Fm2uOpObjectParent(Manager);

	new Fm2uOpTransaction(Manager);

	new Fm2uOpSelection(Manager);

	new Fm2uOpVisibility(Manager);
}
