#pragma once

#include "Styling/ISlateStyle.h"


class Fm2uStyleSet : public FSlateStyleSet
{
public:
	static TSharedPtr<Fm2uStyleSet> Instance;

	Fm2uStyleSet() : FSlateStyleSet("m2uStyleSet")
	{}

	static const ISlateStyle& Get();
	void Initialize();
};
