
#pragma once
#include "Widgets/SCompoundWidget.h"

class SAdvanceDeletionTab : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SAdvanceDeletionTab) {}

	SLATE_ARGUMENT(TArray<TSharedPtr<FAssetData>>, AssetsDataToStore)

	SLATE_ARGUMENT(FString, CurrentSelectedFolder)

	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

private:
	TArray<TSharedPtr<FAssetData>> StoredAssetsData;
	TArray<TSharedPtr<FAssetData>> AssetsDataToDeleteArray;
	TArray<TSharedRef<SCheckBox>> CheckBoxesArray;
	TArray<TSharedPtr<FAssetData>> DisplayedAssetsData;

	TSharedRef<SListView<TSharedPtr<FAssetData>>> ConstructAssetListView();
	TSharedPtr<SListView<TSharedPtr<FAssetData>>> ConstructedAssetListView;
	void RefreshAssetListView();

	// RowWidgetForAssetListView
	TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay, const TSharedRef<STableViewBase>& OwnerTable);
	TSharedRef<SCheckBox> ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay);
	void OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData);
	TSharedRef<STextBlock> ConstructTextForRowWidget(const FString& TextContent, const FSlateFontInfo& FontToUse);
	TSharedRef<SButton> ConstructButtonForRowWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay);
	FReply OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData);
	void OnRowWidgetMoustButtonClicked(TSharedPtr<FAssetData> ClickedData);

	// TabButtons
	TSharedRef<SButton> ConstructDeleteAllButton();
	TSharedRef<SButton> ConstructSelectAllButton();
	TSharedRef<SButton> ConstructDeselectAllButton();

	FReply OnDeleteAllButtonClicked();
	FReply OnSelectAllButtonClicked();
	FReply OnDeselectAllButtonClicked();

	// ComboBoxForListingCondition
	TSharedRef<SComboBox<TSharedPtr<FString>>> ConstructComboBox();
	TArray<TSharedPtr<FString>> ComboBoxSourceItems;
	TSharedRef<SWidget> OnGenerateComboContent(TSharedPtr<FString> SourceItem);
	void OnComboSelectionChanged(TSharedPtr<FString> SelectedOption, ESelectInfo::Type InSelectInfo);
	TSharedPtr<STextBlock> ComboDiplayTextBlock;
	TSharedRef<STextBlock> ConstructComboHelpTexts(const FString& TextContent, ETextJustify::Type TextJustify);

	// etc
	TSharedRef<STextBlock> ConstructTextForTabButtons(const FString& TextContent);
	FSlateFontInfo GetEmboseedTextFont() const { return FCoreStyle::Get().GetFontStyle(FName("EmbossedText")); }
};