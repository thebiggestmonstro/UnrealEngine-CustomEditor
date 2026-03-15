// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FCoreManagerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
    // Content Browser Menu Extention
	void InitContentBrowserMenuExtention();
	TSharedRef<FExtender> CustomContentBrowserMenuExtender(const TArray<FString>& SelectedPaths); 	// Menu Entry의 위치를 설정하는 함수
	void AddContentBrowserMenuEntry(class FMenuBuilder& MenuBuilder);								// Menu Entry의 세부 설정(제목 / 툴팁 / 기능)을 설정하는 함수
	void OnDeleteUnsuedAssetButtonClicked();														// 설정한 기능을 수행하는 함수
	void OnDeleteEmptyFoldersButtonClicked();														// 설정한 기능을 수행하는 함수
	void OnAdvanceDeletionButtonClicked();															// 설정한 기능을 수행하는 함수

	// ContentBrowserMenuExtention
	void PrepareAssetEnvironment(const FString& InSelectedPath);
	void FixUpRedirectors(const FString& InSelectedPath);
	void FixUpRedirectors();
	void SaveWorldIfDirty();

	// CustomEditorTab
	void RegisterAdvanceDeletionTab();
	TSharedRef<SDockTab> OnSpawnAdvanceDeltionTab(const FSpawnTabArgs& SpawnTabArgs);
	TArray<TSharedPtr<FAssetData>> GetAllAssetDataUnderSelectedFolder();

	// LevelEditorExtension
	void InitLevelEditorExtention();
	TSharedRef<FExtender> CustomLevelEditorMenuExtender(const TSharedRef<FUICommandList> UICommandList, const TArray<AActor*> SelectedActors);
	void AddLevelEditorMenuEntry(class FMenuBuilder& MenuBuilder);
	void OnLockActorSelectionButtonClicked();
	void OnUnlockActorSelectionButtonClicked();

	// SelectionLock
	void InitCustomSelectionEvent();
	void OnActorSelected(UObject* SelectedObject);
	void LockActorSelection(AActor* ActorToProcess);
	void UnlockActorSelection(AActor* ActorToProcess);

	TWeakObjectPtr<class UEditorActorSubsystem> WeakEditorActorSubsystem;
	bool GetEditorActorSubsystem();

	// Custom Editor UI Commands
	TSharedPtr<class FUICommandList> CustomUICommands;
	void InitCustomUICommands();
	void OnSelectionLockHotKeyPressed();
	void OnUnlockActorSelectionHotKeyPressed();

	// Scene Outliner Extension
	void InitSceneOutlinerColumnExtension();
	TSharedRef<class ISceneOutlinerColumn> OnCreateSelectionLockColumn(class ISceneOutliner& SceneOutliner);

public:
	// ProccessDataForAdvanceDeletionTab
	bool DeleteSingleAssetForAssetList(const FAssetData& AssetDataToDelete);
	bool DeleteMultipleAssetsForAssetList(const TArray<FAssetData>& AssetsToDelete);
	void ListUnusedAssetsForAssetList(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter, TArray<TSharedPtr<FAssetData>>& OutUnusedAssetsData);
	void ListSameNameAssetsForAssetList(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter, TArray<TSharedPtr<FAssetData>>& OutSameNameAssetsData);
	void SyncContentBrowserToClickedAssetForAssetList(const FString& AssetPathToSync);

	// Scene Outliner
	bool CheckIsActorSelectionLocked(AActor* ActorToProcess);
	void ProcessLockingForOutliner(AActor* ActorToProcess, bool bShouldLock);
	void RefreshSceneOutliner();

	TArray<FString> FolderPathsSelected;
};
