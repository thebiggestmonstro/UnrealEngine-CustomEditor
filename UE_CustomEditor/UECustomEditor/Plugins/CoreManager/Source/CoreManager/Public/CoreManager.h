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
	// Menu Entry의 위치를 설정하는 함수
	TSharedRef<FExtender> CustomContentBrowserMenuExtender(const TArray<FString>& SelectedPaths);
	// Menu Entry의 세부 설정(제목 / 툴팁 / 기능)을 설정하는 함수
	void AddContentBrowserMenuEntry(class FMenuBuilder& MenuBuilder);
	// 설정한 기능을 수행하는 함수
	void OnDeleteUnsuedAssetButtonClicked();
	void OnDeleteEmptyFoldersButtonClicked();
	void OnAdvanceDeletionButtonClicked();

	// ContentBrowserMenuExtention
	void PrepareAssetEnvironment(const FString& InSelectedPath);
	void FixUpRedirectors(const FString& InSelectedPath);
	void FixUpRedirectors();
	void SaveWorldIfDirty();

	// CustomEditorTab
	void RegisterAdvanceDeletionTab();
	TSharedRef<SDockTab> OnSpawnAdvanceDeltionTab(const FSpawnTabArgs& SpawnTabArgs);
	TArray<TSharedPtr<FAssetData>> GetAllAssetDataUnderSelectedFolder();

public:
	// ProccessDataForAdvanceDeletionTab
	bool DeleteSingleAssetForAssetList(const FAssetData& AssetDataToDelete);
	bool DeleteMultipleAssetsForAssetList(const TArray<FAssetData>& AssetsToDelete);
	void ListUnusedAssetsForAssetList(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter, TArray<TSharedPtr<FAssetData>>& OutUnusedAssetsData);
	void ListSameNameAssetsForAssetList(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter, TArray<TSharedPtr<FAssetData>>& OutSameNameAssetsData);

	TArray<FString> FolderPathsSelected;
};
