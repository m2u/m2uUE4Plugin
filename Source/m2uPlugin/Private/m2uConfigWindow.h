#pragma once

class Sm2uConfigWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(Sm2uConfigWindow){}
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);
	FReply DoResetConnection();
	void OnPortTextChanged(const FText& InText, ETextCommit::Type InCommitType);
	FText GetPortText() const;

private:
	FString PortString;
	
};
