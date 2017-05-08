#include "m2uPluginPrivatePCH.h"
#include "m2uStyleSet.h"
#include "IPluginManager.h"


TSharedPtr<Fm2uStyleSet> Fm2uStyleSet::Instance;


const ISlateStyle& Fm2uStyleSet::Get() {
	if (! Instance.IsValid()) {
		TSharedRef< class Fm2uStyleSet > NewStyle = MakeShareable(new Fm2uStyleSet());
		Instance = NewStyle;
		Instance->Initialize();
		FSlateStyleRegistry::RegisterSlateStyle(*Instance);
	}
	return *Instance;
}

void Fm2uStyleSet::Initialize() {
	const FVector2D Icon16x16(16.0f, 16.0f);

	FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("m2uPlugin"))->GetContentDir();
	SetContentRoot(ContentDir);

	Set("Icons.m2u", new FSlateImageBrush(RootToContentDir("m2uIcon16", TEXT(".png")), Icon16x16));
}
