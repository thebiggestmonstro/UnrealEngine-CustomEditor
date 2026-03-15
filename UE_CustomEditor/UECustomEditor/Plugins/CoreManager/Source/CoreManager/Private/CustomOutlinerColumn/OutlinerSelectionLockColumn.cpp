#include "CustomOutlinerColumn/OutlinerSelectionLockColumn.h"
#include "CustomStyle/CoreManagerStyle.h"
#include "ActorTreeItem.h"
#include "CoreManager.h"

SHeaderRow::FColumn::FArguments FOutlinerSelectionLockColumn::ConstructHeaderRowColumn()
{
	SHeaderRow::FColumn::FArguments ConstructedHeaderRow =
		SHeaderRow::Column(GetColumnID())
		.FixedWidth(24.f)
		.HAlignHeader(HAlign_Center)
		.VAlignHeader(VAlign_Center)
		.HAlignCell(HAlign_Center)
		.VAlignCell(VAlign_Center)
		.DefaultTooltip(FText::FromString(TEXT("Actor Selection Lock - Press icon to lock actor selection")))
		[
			SNew(SImage)
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Image(FCoreManagerStyle::GetCreatedSlateStyleSet()->GetBrush(FName("LevelEditor.LockSelection")))
		];

	return ConstructedHeaderRow;
}

const TSharedRef<SWidget> FOutlinerSelectionLockColumn::ConstructRowWidget(FSceneOutlinerTreeItemRef TreeItem, const STableRow<FSceneOutlinerTreeItemPtr>& Row)
{
	FActorTreeItem* ActorTreeItem = TreeItem->CastTo<FActorTreeItem>();

	if (!ActorTreeItem || !ActorTreeItem->IsValid())
	{
		return SNullWidget::NullWidget;
	}

	FCoreManagerModule& CoreManagerModule = FModuleManager::LoadModuleChecked<FCoreManagerModule>(TEXT("CoreManager"));
	const bool bIsActorSelectionLocked = CoreManagerModule.CheckIsActorSelectionLocked(ActorTreeItem->Actor.Get());

	const FCheckBoxStyle& ToggleButtonStyle = FCoreManagerStyle::GetCreatedSlateStyleSet()->GetWidgetStyle<FCheckBoxStyle>(FName("SceneOutliner.SelectionLock"));

	TSharedRef<SCheckBox> ConstructedRowWidgetCheckBox =
		SNew(SCheckBox)
		.Visibility(EVisibility::Visible)
		.Type(ESlateCheckBoxType::ToggleButton)
		.Style(&ToggleButtonStyle)
		.HAlign(HAlign_Center)
		.IsChecked(bIsActorSelectionLocked ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
		.OnCheckStateChanged(this, &FOutlinerSelectionLockColumn::OnRowWdigetCheckStateChanged, ActorTreeItem->Actor);

	return ConstructedRowWidgetCheckBox;
}

void FOutlinerSelectionLockColumn::OnRowWdigetCheckStateChanged(ECheckBoxState NewState, TWeakObjectPtr<AActor> CorrespondingActor)
{
	FCoreManagerModule& CoreManagerModule = FModuleManager::LoadModuleChecked<FCoreManagerModule>(TEXT("CoreManager"));

	switch (NewState)
	{
	case ECheckBoxState::Unchecked:
		CoreManagerModule.ProcessLockingForOutliner(CorrespondingActor.Get(), false);
		break;
	case ECheckBoxState::Checked:
		CoreManagerModule.ProcessLockingForOutliner(CorrespondingActor.Get(), true);
		break;
	case ECheckBoxState::Undetermined:
		break;
	default:
		break;
	}
}