// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoreManager.h"
#include "ContentBrowserModule.h"
#include "DebugHeader.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "IContentBrowserSingleton.h"
#include "LevelEditorSubsystem.h"
#include "SlateWidgets/AdvanceDeletionWidget.h"
#include "CustomStyle/CoreManagerStyle.h"
#include "LevelEditor.h"
#include "Engine/Selection.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "CustomUICommands/CoreManagerUICommands.h"
#include "SceneOutlinerModule.h"
#include "CustomOutlinerColumn/OutlinerSelectionLockColumn.h"

#define LOCTEXT_NAMESPACE "FCoreManagerModule"

void FCoreManagerModule::StartupModule()
{
	FCoreManagerStyle::InitializeIcons();
	InitContentBrowserMenuExtention();
	RegisterAdvanceDeletionTab();

	FCoreManagerUICommands::Register();
	InitCustomUICommands();

	InitLevelEditorExtention();
	InitCustomSelectionEvent();

	InitSceneOutlinerColumnExtension();
}

void FCoreManagerModule::ShutdownModule()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FName("AdvanceDeletion"));
	FCoreManagerStyle::ShutDown();
}

void FCoreManagerModule::InitContentBrowserMenuExtention()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	//Get hold of all the menu extenders
	TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBrowserModuleMenuExtenders = ContentBrowserModule.GetAllPathViewContextMenuExtenders();

	FContentBrowserMenuExtender_SelectedPaths CustomContentBrowserMenuDelegate;

	//We add custom delete to all the existing delegate
	ContentBrowserModuleMenuExtenders.Add(FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FCoreManagerModule::CustomContentBrowserMenuExtender));
}

//To define the positio for inserting menu entry
TSharedRef<FExtender> FCoreManagerModule::CustomContentBrowserMenuExtender(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> MenuExtender(new FExtender());

	if (SelectedPaths.Num() > 0)
	{
		MenuExtender->AddMenuExtension(FName("Delete"), //Extend hook, position to insert
			EExtensionHook::After,						//Insert before or after
			TSharedPtr<FUICommandList>(),				//Custom hot keys 
			FMenuExtensionDelegate::CreateRaw(this, &FCoreManagerModule::AddContentBrowserMenuEntry)); //Second binding, will define details for this menu entry

		FolderPathsSelected = SelectedPaths;
	}

	return MenuExtender;
}

//Define details for the custom menu entry
void FCoreManagerModule::AddContentBrowserMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Delete Unused Assets")),							//Title text for menu entry
		FText::FromString(TEXT("Safely delete all unused assets under folder")),	//Tooltip text
		FSlateIcon(FCoreManagerStyle::GetStyleSetName(), "ContentBrowser.DeleteUnusedAssets"),	//Custom icon
		FExecuteAction::CreateRaw(this, &FCoreManagerModule::OnDeleteUnsuedAssetButtonClicked) //The actual function to excute
	);

	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Delete Empty Folders")), //Title text for menu entry
		FText::FromString(TEXT("Safely delete all empty folders")), //Tooltip text
		FSlateIcon(FCoreManagerStyle::GetStyleSetName(), "ContentBrowser.DeleteEmptyFolders"),	//Custom icon
		FExecuteAction::CreateRaw(this, &FCoreManagerModule::OnDeleteEmptyFoldersButtonClicked) //The actual function to excute
	);

	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Advance Deletion")), //Title text for menu entry
		FText::FromString(TEXT("List assets by specific condition in a tab for deleting")), //Tooltip text
		FSlateIcon(FCoreManagerStyle::GetStyleSetName(), "ContentBrowser.AdvanceDeletion"),	//Custom icon
		FExecuteAction::CreateRaw(this, &FCoreManagerModule::OnAdvanceDeletionButtonClicked) //The actual function to excute
	);
}

void FCoreManagerModule::OnDeleteUnsuedAssetButtonClicked()
{
	if (FolderPathsSelected.Num() > 1)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("You can only do this to one folder"));
		return;
	}

	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);

	if (AssetsPathNames.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No asset found under selected folder"), false);
		return;
	}

	EAppReturnType::Type ConfirmResult = DebugHeader::ShowMsgDialog(EAppMsgType::YesNo, TEXT("A total of ") + FString::FromInt(AssetsPathNames.Num())
		+ TEXT(" assets need to be checked.\nWould you like to procceed?"), false);
	if (ConfirmResult == EAppReturnType::No)
	{
		return;
	}

	PrepareAssetEnvironment(FolderPathsSelected[0]);

	TArray<FAssetData> UnusedAssetsDataArray;

	for (const FString& AssetPathName : AssetsPathNames)
	{
		if (AssetPathName.Contains(TEXT("Developers")) || AssetPathName.Contains(TEXT("Collections")) ||
			AssetPathName.Contains(TEXT("__ExternalActors__")) ||
			AssetPathName.Contains(TEXT("__ExternalObjects__")))
		{
			continue;
		}

		if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName))
		{
			continue;
		}

		TArray<FString> AssetReferencers = UEditorAssetLibrary::FindPackageReferencersForAsset(AssetPathName);

		if (AssetReferencers.Num() == 0)
		{
			const FAssetData UnusedAssetData = UEditorAssetLibrary::FindAssetData(AssetPathName);
			UnusedAssetsDataArray.Add(UnusedAssetData);
		}
	}

	if (UnusedAssetsDataArray.Num() > 0)
	{
		ObjectTools::DeleteAssets(UnusedAssetsDataArray);
	}
	else
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No unused asset found under selected folder"), false);
	}
}

void FCoreManagerModule::OnDeleteEmptyFoldersButtonClicked()
{
	PrepareAssetEnvironment(FolderPathsSelected[0]);

	TArray<FString> FolderPathsArray = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0], true, true);
	uint32 Counter = 0;

	FString EmptyFolderPathsNames;
	TArray<FString> EmptyFoldersPathsArray;

	for (const FString& FolderPath : FolderPathsArray)
	{
		if (FolderPath.Contains(TEXT("Developers")) ||
			FolderPath.Contains(TEXT("Collections")) ||
			FolderPath.Contains(TEXT("__ExternalActors__")) ||
			FolderPath.Contains(TEXT("__ExternalObjects__")))
		{
			continue;
		}

		if (!UEditorAssetLibrary::DoesDirectoryExist(FolderPath))
		{
			continue;
		}

		FString Path = FolderPath;
		FPaths::NormalizeDirectoryName(Path);
		Path.RemoveFromEnd(TEXT("/"));
		if (!UEditorAssetLibrary::DoesDirectoryHaveAssets(Path, false))
		{
			EmptyFolderPathsNames += Path + TEXT("\n");
			EmptyFoldersPathsArray.Add(Path);
		}
		
	}

	if (EmptyFoldersPathsArray.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No empty folder found under selected folder"), false);
		return;
	}

	EAppReturnType::Type ConfirmResult = DebugHeader::ShowMsgDialog(EAppMsgType::OkCancel,
		TEXT("Empty folders found in:\n") + EmptyFolderPathsNames + TEXT("\nWould you like to delete all?"), false);

	if (ConfirmResult == EAppReturnType::Cancel)
	{
		return;
	}

	for (const FString& EmptyFolderPath : EmptyFoldersPathsArray)
	{
		if (UEditorAssetLibrary::DeleteDirectory(EmptyFolderPath))
		{
			++Counter;
		}
		else
		{
			DebugHeader::Print(TEXT("Failed to delete " + EmptyFolderPath), FColor::Red);
		}
	}

	if (Counter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Successfully deleted ") + FString::FromInt(Counter) + TEXT(" folders"));
	}
}

void FCoreManagerModule::OnAdvanceDeletionButtonClicked()
{
	FixUpRedirectors();
	FGlobalTabmanager::Get()->TryInvokeTab(FName("AdvanceDeletion"));
}

void FCoreManagerModule::PrepareAssetEnvironment(const FString& InSelectedPath)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetDataList;

	AssetRegistryModule.Get().GetAssetsByPath(FName(*InSelectedPath), AssetDataList, true);

	FixUpRedirectors(InSelectedPath);

	for (const FAssetData& AssetData : AssetDataList)
	{
		UEditorAssetLibrary::SaveAsset(AssetData.GetSoftObjectPath().ToString(), false);
	}

	SaveWorldIfDirty();
}

void FCoreManagerModule::FixUpRedirectors(const FString& InSelectedPath)
{
	TArray<UObjectRedirector*> RedirectorsToFixArray;

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace(*InSelectedPath);
	Filter.ClassPaths.Add(UObjectRedirector::StaticClass()->GetClassPathName());

	TArray<FAssetData> OutRedirectors;
	AssetRegistryModule.Get().GetAssets(Filter, OutRedirectors);

	for (const FAssetData& RedirectorData : OutRedirectors)
	{
		if (UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(RedirectorData.GetAsset()))
		{
			RedirectorsToFixArray.Add(RedirectorToFix);
		}
	}

	if (RedirectorsToFixArray.Num() > 0)
	{
		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
		AssetToolsModule.Get().FixupReferencers(RedirectorsToFixArray);
	}
}

void FCoreManagerModule::FixUpRedirectors()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FString> SelectedPaths;
	ContentBrowserModule.Get().SetSelectedPaths(SelectedPaths);

	if (SelectedPaths.Num() == 0)
	{
		return;
	}

	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.ClassPaths.Add(UObjectRedirector::StaticClass()->GetClassPathName());

	for (const FString& Path : SelectedPaths)
	{
		Filter.PackagePaths.Add(*Path);
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	TArray<FAssetData> OutRedirectors;
	AssetRegistryModule.Get().GetAssets(Filter, OutRedirectors);

	TArray<UObjectRedirector*> RedirectorsToFixArray;
	for (const FAssetData& RedirectorData : OutRedirectors)
	{
		if (UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(RedirectorData.GetAsset()))
		{
			RedirectorsToFixArray.Add(RedirectorToFix);
		}
	}

	if (RedirectorsToFixArray.Num() > 0)
	{
		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
		AssetToolsModule.Get().FixupReferencers(RedirectorsToFixArray);
	}
}

void FCoreManagerModule::SaveWorldIfDirty()
{
	if (GEditor)
	{
		if (ULevelEditorSubsystem* LevelEditorSubsystem = GEditor->GetEditorSubsystem<ULevelEditorSubsystem>())
		{
			if (UWorld* World = GEditor->GetEditorWorldContext().World())
			{
				if (ULevel* CurrentLevel = World->GetCurrentLevel())
				{
					UPackage* DirtyMapPackage = CurrentLevel->GetOutermost();
					if (DirtyMapPackage->IsDirty())
					{
						LevelEditorSubsystem->SaveCurrentLevel();
					}
				}
			}
		}
	}
}

void FCoreManagerModule::RegisterAdvanceDeletionTab()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("AdvanceDeletion")
		,FOnSpawnTab::CreateRaw(this, &FCoreManagerModule::OnSpawnAdvanceDeltionTab))
		.SetDisplayName(FText::FromString(TEXT("Advance Deletion")))
		.SetIcon(FSlateIcon(FCoreManagerStyle::GetStyleSetName(), "ContentBrowser.AdvanceDeletion")
	);
}

TSharedRef<SDockTab> FCoreManagerModule::OnSpawnAdvanceDeltionTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab).TabRole(ETabRole::NomadTab)
		[
			SNew(SAdvanceDeletionTab)
				.AssetsDataToStore(GetAllAssetDataUnderSelectedFolder())
				.CurrentSelectedFolder(FolderPathsSelected[0])
		];
}

TArray<TSharedPtr<FAssetData>> FCoreManagerModule::GetAllAssetDataUnderSelectedFolder()
{
	TArray< TSharedPtr <FAssetData> > AvaiableAssetsData;
	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);

	for (const FString& AssetPathName : AssetsPathNames)
	{
		//Don't touch root folder
		if (AssetPathName.Contains(TEXT("Developers")) ||
			AssetPathName.Contains(TEXT("Collections")) ||
			AssetPathName.Contains(TEXT("__ExternalActors__")) ||
			AssetPathName.Contains(TEXT("__ExternalObjects__")))
		{
			continue;
		}

		if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName))
		{
			continue;
		}

		const FAssetData Data = UEditorAssetLibrary::FindAssetData(AssetPathName);
		AvaiableAssetsData.Add(MakeShared<FAssetData>(Data));
	}

	return AvaiableAssetsData;
}

void FCoreManagerModule::InitLevelEditorExtention()
{
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));

	TSharedRef<FUICommandList> ExistingLevelCommands = LevelEditorModule.GetGlobalLevelEditorActions();
	ExistingLevelCommands->Append(CustomUICommands.ToSharedRef());

	TArray<FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors>& LevelEditorMenuExtenders = LevelEditorModule.GetAllLevelViewportContextMenuExtenders();

	LevelEditorMenuExtenders.Add(FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors::CreateRaw(this, &FCoreManagerModule::CustomLevelEditorMenuExtender));
}

TSharedRef<FExtender> FCoreManagerModule::CustomLevelEditorMenuExtender(const TSharedRef<FUICommandList> UICommandList, const TArray<AActor*> SelectedActors)
{
	TSharedRef<FExtender> MenuExtender = MakeShareable(new FExtender());

	if (SelectedActors.Num() > 0)
	{
		MenuExtender->AddMenuExtension(
			FName("ActorOptions"),
			EExtensionHook::Before,
			UICommandList,
			FMenuExtensionDelegate::CreateRaw(this, &FCoreManagerModule::AddLevelEditorMenuEntry)
		);
	}

	return MenuExtender;
}

void FCoreManagerModule::AddLevelEditorMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Lock Actor Selection")),
		FText::FromString(TEXT("Prevent actor from being selected")),
		FSlateIcon(FCoreManagerStyle::GetStyleSetName(), "LevelEditor.LockSelection"),
		FExecuteAction::CreateRaw(this, &FCoreManagerModule::OnLockActorSelectionButtonClicked)
	);

	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Unlock all actor Selection")),
		FText::FromString(TEXT("Remove the selection constraint on all actor")),
		FSlateIcon(FCoreManagerStyle::GetStyleSetName(), "LevelEditor.UnlockSelection"),
		FExecuteAction::CreateRaw(this, &FCoreManagerModule::OnUnlockActorSelectionButtonClicked)
	);
}

void FCoreManagerModule::OnLockActorSelectionButtonClicked()
{
	if (!GetEditorActorSubsystem())
	{
		return;
	}

	TArray<AActor*> SelectedActors = WeakEditorActorSubsystem->GetSelectedLevelActors();

	if (SelectedActors.Num() == 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("No actor selected"));
		return;
	}

	FString CurrentLockedActorNames = TEXT("Locked selection for:");

	for (AActor* SelectedActor : SelectedActors)
	{
		if (!SelectedActor)
		{
			continue;
		}

		LockActorSelection(SelectedActor);

		WeakEditorActorSubsystem->SetActorSelectionState(SelectedActor, false);

		CurrentLockedActorNames.Append(TEXT("\n"));
		CurrentLockedActorNames.Append(SelectedActor->GetActorLabel());
	}

	DebugHeader::ShowNotifyInfo(CurrentLockedActorNames);
}

void FCoreManagerModule::OnUnlockActorSelectionButtonClicked()
{
	if (!GetEditorActorSubsystem())
	{
		return;
	}

	TArray<AActor*> AllActorsInLevel = WeakEditorActorSubsystem->GetAllLevelActors();
	TArray<AActor*> AllLockedActors;

	for (AActor* ActorInLevel : AllActorsInLevel)
	{
		if (!ActorInLevel)
		{
			continue;
		}

		if (CheckIsActorSelectionLocked(ActorInLevel))
		{
			AllLockedActors.Add(ActorInLevel);
		}
	}

	if (AllLockedActors.Num() == 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("No selection locked actor currently"));
	}

	FString UnlockedActorNames = TEXT("Lifted selection constraint for:");

	for (AActor* LockedActor : AllLockedActors)
	{
		UnlockActorSelection(LockedActor);

		UnlockedActorNames.Append(TEXT("\n"));
		UnlockedActorNames.Append(LockedActor->GetActorLabel());
	}

	DebugHeader::ShowNotifyInfo(UnlockedActorNames);
}

void FCoreManagerModule::InitCustomSelectionEvent()
{
	USelection* UserSelection = GEditor->GetSelectedActors();
	UserSelection->SelectObjectEvent.AddRaw(this, &FCoreManagerModule::OnActorSelected);
}

void FCoreManagerModule::OnActorSelected(UObject* SelectedObject)
{
	if (!GetEditorActorSubsystem())
	{
		return;
	}

	if (AActor* SelectedActor = Cast<AActor>(SelectedObject))
	{
		if (CheckIsActorSelectionLocked(SelectedActor))
		{
			//Deselect actor right away
			WeakEditorActorSubsystem->SetActorSelectionState(SelectedActor, false);
		}
	}
}

void FCoreManagerModule::LockActorSelection(AActor* ActorToProcess)
{
	if (!ActorToProcess)
	{
		return;
	}

	if (!ActorToProcess->ActorHasTag(FName("Locked")))
	{
		ActorToProcess->Tags.Add(FName("Locked"));
	}
}

void FCoreManagerModule::UnlockActorSelection(AActor* ActorToProcess)
{
	if (!ActorToProcess)
	{
		return;
	}

	if (ActorToProcess->ActorHasTag(FName("Locked")))
	{
		ActorToProcess->Tags.Remove(FName("Locked"));
	}
}

bool FCoreManagerModule::CheckIsActorSelectionLocked(AActor* ActorToProcess)
{
	if (!ActorToProcess)
	{
		return false;
	}

	return ActorToProcess->ActorHasTag(FName("Locked"));
}

bool FCoreManagerModule::GetEditorActorSubsystem()
{
	if (!WeakEditorActorSubsystem.IsValid())
	{
		WeakEditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
	}

	return WeakEditorActorSubsystem.IsValid();
}

void FCoreManagerModule::InitCustomUICommands()
{
	CustomUICommands = MakeShareable(new FUICommandList());

	CustomUICommands->MapAction(
		FCoreManagerUICommands::Get().LockActorSelection,
		FExecuteAction::CreateRaw(this, &FCoreManagerModule::OnSelectionLockHotKeyPressed)
	);

	CustomUICommands->MapAction(
		FCoreManagerUICommands::Get().UnlockActorSelection,
		FExecuteAction::CreateRaw(this, &FCoreManagerModule::OnUnlockActorSelectionHotKeyPressed)
	);
}

void FCoreManagerModule::OnSelectionLockHotKeyPressed()
{
	OnLockActorSelectionButtonClicked();
}

void FCoreManagerModule::OnUnlockActorSelectionHotKeyPressed()
{
	OnUnlockActorSelectionButtonClicked();
}

void FCoreManagerModule::InitSceneOutlinerColumnExtension()
{
	FSceneOutlinerModule& SceneOutlinerModule = FModuleManager::LoadModuleChecked<FSceneOutlinerModule>(TEXT("SceneOutliner"));

	FSceneOutlinerColumnInfo SelectionLockColumnInfo(
		ESceneOutlinerColumnVisibility::Visible,
		1,
		FCreateSceneOutlinerColumn::CreateRaw(this, &FCoreManagerModule::OnCreateSelectionLockColumn)
	);

	SceneOutlinerModule.RegisterDefaultColumnType<FOutlinerSelectionLockColumn>(SelectionLockColumnInfo);
}

TSharedRef<ISceneOutlinerColumn> FCoreManagerModule::OnCreateSelectionLockColumn(ISceneOutliner& SceneOutliner)
{
	return MakeShareable(new FOutlinerSelectionLockColumn(SceneOutliner));
}

bool FCoreManagerModule::DeleteSingleAssetForAssetList(const FAssetData& AssetDataToDelete)
{
	TArray<FAssetData> AssetDataForDeletion;
	AssetDataForDeletion.Add(AssetDataToDelete);

	if (ObjectTools::DeleteAssets(AssetDataForDeletion) > 0)
	{
		return true;
	}

	return false;
}

bool FCoreManagerModule::DeleteMultipleAssetsForAssetList(const TArray<FAssetData>& AssetsToDelete)
{
	if (ObjectTools::DeleteAssets(AssetsToDelete) > 0)
	{
		return true;
	}

	return false;
}

void FCoreManagerModule::ListUnusedAssetsForAssetList(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter, TArray<TSharedPtr<FAssetData>>& OutUnusedAssetsData)
{
	SaveWorldIfDirty();

	OutUnusedAssetsData.Empty();

	for (const TSharedPtr<FAssetData>& DataSharedPtr : AssetsDataToFilter)
	{
		TArray<FString> AssetReferencers = UEditorAssetLibrary::FindPackageReferencersForAsset(DataSharedPtr->ObjectPath.ToString());
		if (AssetReferencers.Num() == 0)
		{
			OutUnusedAssetsData.Add(DataSharedPtr);
		}
	}
}

void FCoreManagerModule::ListSameNameAssetsForAssetList(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter, TArray<TSharedPtr<FAssetData>>& OutSameNameAssetsData)
{
	OutSameNameAssetsData.Empty();

	//Multimap for supporting finding assets with same name
	TMultiMap<FString, TSharedPtr<FAssetData>> AssetsInfoMultiMap;

	for (const TSharedPtr<FAssetData>& DataSharedPtr : AssetsDataToFilter)
	{
		AssetsInfoMultiMap.Emplace(DataSharedPtr->AssetName.ToString(), DataSharedPtr);
	}

	for (const TSharedPtr<FAssetData>& DataSharedPtr : AssetsDataToFilter)
	{
		TArray< TSharedPtr <FAssetData> > OutAssetsData;
		AssetsInfoMultiMap.MultiFind(DataSharedPtr->AssetName.ToString(), OutAssetsData);

		if (OutAssetsData.Num() <= 1)
		{
			continue;
		}

		for (const TSharedPtr<FAssetData>& SameNameData : OutAssetsData)
		{
			if (SameNameData.IsValid())
			{
				OutSameNameAssetsData.AddUnique(SameNameData);
			}
		}
	}
}

void FCoreManagerModule::SyncContentBrowserToClickedAssetForAssetList(const FString& AssetPathToSync)
{
	TArray<FString> AssetsPathToSync;
	AssetsPathToSync.Add(AssetPathToSync);
	UEditorAssetLibrary::SyncBrowserToObjects(AssetsPathToSync);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCoreManagerModule, CoreManager)