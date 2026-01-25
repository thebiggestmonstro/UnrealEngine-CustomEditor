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

	void FixUpRedirectors(const FString& InSelectedPath);

	TArray<FString> FolderPathsSelected;
};
