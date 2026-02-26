#include "CustomStyle/CoreManagerStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"

FName FCoreManagerStyle::StyleSetName = FName("CoreManagerStyle");
TSharedPtr<FSlateStyleSet> FCoreManagerStyle::CreatedSlateStyleSet = nullptr;

void FCoreManagerStyle::InitializeIcons()
{
	if (!CreatedSlateStyleSet.IsValid())
	{
		CreatedSlateStyleSet = CreateSlateStyleSet();
		FSlateStyleRegistry::RegisterSlateStyle(*CreatedSlateStyleSet);
	}

}

TSharedRef<FSlateStyleSet> FCoreManagerStyle::CreateSlateStyleSet()
{
	TSharedRef<FSlateStyleSet> CustomStyleSet = MakeShareable(new FSlateStyleSet(StyleSetName));
	const FString IconDirectory = IPluginManager::Get().FindPlugin(TEXT("CoreManager"))->GetBaseDir() / "Resources";
	CustomStyleSet->SetContentRoot(IconDirectory);

	const FVector2D Icon16x16(16.f, 16.f);

	CustomStyleSet->Set("ContentBrowser.DeleteUnusedAssets", new FSlateImageBrush(IconDirectory / "DeleteUnusedAsset.png", Icon16x16));
	CustomStyleSet->Set("ContentBrowser.DeleteEmptyFolders", new FSlateImageBrush(IconDirectory / "DeleteEmptyFolders.png", Icon16x16));
	CustomStyleSet->Set("ContentBrowser.AdvanceDeletion", new FSlateImageBrush(IconDirectory / "AdvanceDeletion.png", Icon16x16));

	return CustomStyleSet;
}

void FCoreManagerStyle::ShutDown()
{
	if (CreatedSlateStyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*CreatedSlateStyleSet);
		CreatedSlateStyleSet.Reset();
	}
}