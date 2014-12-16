#include "m2uPluginPrivatePCH.h"
#include "m2uUI.h"
#include "m2uConfigWindow.h"

#define LOCTEXT_NAMESPACE "m2u"
namespace m2uUI
{
	static const FName m2uTabName = FName( TEXT("m2u") );

	TSharedRef<SDockTab> SpawnTab( const FSpawnTabArgs& SpawnTabArgs )
	{
		const TSharedRef<SDockTab> Tab =
			SNew( SDockTab )
			.Icon( FEditorStyle::GetBrush("LiveEditor.TabIcon") )
			.TabRole( ETabRole::NomadTab );

		Tab->SetContent( SNew( Sm2uConfigWindow ) );

		return Tab;
	}

	void RegisterUI()
	{
		// Register a tab spawner so that our tab can be automatically restored from layout files
		FGlobalTabmanager::Get()->RegisterNomadTabSpawner( m2uTabName, FOnSpawnTab::CreateStatic( &SpawnTab ) )
			.SetDisplayName( LOCTEXT( "m2uTabTitle", "m2u" ) )
			.SetTooltipText( LOCTEXT( "m2uTooltipText", "Open the m2u window." ))
			//.SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsCategory())
			.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LiveEditor.TabIcon"));
	}

	void UnregisterUI()
	{
		// Unregister the tab spawner
		FGlobalTabmanager::Get()->UnregisterTabSpawner( m2uTabName );
	}
}

#undef LOCTEXT_NAMESPACE
