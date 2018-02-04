#include "m2uConfigWindow.h"
#include "m2uPlugin.h"

DEFINE_LOG_CATEGORY_STATIC(m2uConfigWindow, Log, All);

void Sm2uConfigWindow::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew( SOverlay )
		+SOverlay::Slot()
		[
			SNew( SVerticalBox )
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2.0f)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f)
				[
					SNew(SButton)
					.Text( FText::FromString(FString(TEXT("Reset Connection"))) )
					.OnClicked( this, &Sm2uConfigWindow::DoResetConnection)
				]
				+SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f)
				[
					SNew(SEditableTextBox)
					.SelectAllTextWhenFocused(true)
					.Text(this, &Sm2uConfigWindow::GetPortText)
					.ToolTipText( FText::FromString(FString(TEXT("The Port on which to listen."))) )
					.OnTextCommitted(this, &Sm2uConfigWindow::OnPortTextChanged)
					.OnTextChanged(this, &Sm2uConfigWindow::OnPortTextChanged, ETextCommit::Default)
				]
			]
		]
	];
}

FReply Sm2uConfigWindow::DoResetConnection()
{
	uint16 Port = DEFAULT_M2U_PORT;
	if( PortString.Len()>0 )
		Port = FCString::Atoi(*PortString);

	Fm2uPlugin& Connection = Fm2uPlugin::Get();
	Connection.ResetConnection(Port);
	return FReply::Handled();
}

void Sm2uConfigWindow::OnPortTextChanged(const FText& InText, ETextCommit::Type InCommitType)
{
	PortString = InText.ToString();
}

FText Sm2uConfigWindow::GetPortText() const
{
	return ( PortString != FString(TEXT("")) )? FText::FromString(PortString) : FText::FromString(FString::Printf(TEXT("%i"),DEFAULT_M2U_PORT));
}
