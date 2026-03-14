#pragma once

#include "Styling/SlateStyle.h"

class FCoreManagerStyle
{
public:
	static void InitializeIcons();
	static void ShutDown();

private:
	static FName StyleSetName;
	static TSharedRef<FSlateStyleSet> CreateSlateStyleSet();
	static TSharedPtr<FSlateStyleSet> CreatedSlateStyleSet;

public:
	static FName GetStyleSetName() { return StyleSetName; }
	static TSharedRef<FSlateStyleSet> GetCreatedSlateStyleSet() { return CreatedSlateStyleSet.ToSharedRef(); }
};