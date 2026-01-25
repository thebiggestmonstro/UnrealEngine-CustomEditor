// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetActions/QuickAssetActions.h"
#include "DebugHeader.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetToolsModule.h"
#include "AssetViewUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"

void UQuickAssetActions::DuplicateAssets(int32 NumOfDuplicates)
{
	if (NumOfDuplicates <= 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Please enter a VALID number"));
		return;
	}

	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	uint32 Counter = 0;

	for (const FAssetData& SelectedAssetData : SelectedAssetsData)
	{
		for (int32 i = 0; i < NumOfDuplicates; i++)
		{
			const FString SourceAssetPath = SelectedAssetData.ObjectPath.ToString();
			const FString NewDuplicateAssetNamae = SelectedAssetData.AssetName.ToString() + TEXT("_") + FString::FromInt(i + 1);
			const FString NewPathName = FPaths::Combine(SelectedAssetData.PackagePath.ToString(), NewDuplicateAssetNamae);

			if (UEditorAssetLibrary::DuplicateAsset(SourceAssetPath, NewPathName))
			{
				UEditorAssetLibrary::SaveAsset(NewPathName, false);
				++Counter;
			}
		}
	}

	if (Counter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Successfully duplicated " + FString::FromInt(Counter) + " files"));
	}
}

void UQuickAssetActions::AddPrefixes()
{
	TArray<UObject*>SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();
	uint32 Counter = 0;

	for (UObject* SelectedObject : SelectedObjects)
	{
		if (!SelectedObject)
		{
			continue;
		}

		FString* PrefixFound = PrefixMap.Find(SelectedObject->GetClass());
		FString OldName = SelectedObject->GetName();

		if (!PrefixFound || PrefixFound->IsEmpty())
		{
			DebugHeader::Print(TEXT("Failed to find prefix for class ") + SelectedObject->GetClass()->GetName(), FColor::Red);
			continue;
		}

		if (OldName.StartsWith(*PrefixFound))
		{
			DebugHeader::Print(OldName + TEXT(" already has prefix added"), FColor::Red);
			continue;
		}

		if (SelectedObject->IsA<UMaterialInstanceConstant>())
		{
			OldName.RemoveFromStart(TEXT("M_"));
			OldName.RemoveFromEnd(TEXT("_Inst"));
		}

		const FString NewNameWithPrefix = *PrefixFound + OldName;
		UEditorUtilityLibrary::RenameAsset(SelectedObject, NewNameWithPrefix);

		if (SelectedObject->GetName() == NewNameWithPrefix)
		{
			++Counter;
		}
	}

	if (Counter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Successfully renamed " + FString::FromInt(Counter) + " assets"));
	}
}

void UQuickAssetActions::RemoveUnusedAssets()
{
	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<FAssetData> UnusedAssetsData;

	// 선택한 애셋들의 리디렉터 파일 수정
	TArray<FName> PathsToFix = GetTopLevelPackagePath(SelectedAssetsData);
	FixUpRedirectors(PathsToFix);

	// 선택한 애셋 데이터들을 순회
	for (const FAssetData& SelectedAssetData : SelectedAssetsData)
	{
		TArray<FString> AssetRefrencers = UEditorAssetLibrary::FindPackageReferencersForAsset(SelectedAssetData.ObjectPath.ToString());

		// 선택한 애셋 데이터들중에서 참조되고 있지 않은 애셋들은 제거를 위한 컨테이너에 추기
		if (AssetRefrencers.Num() == 0)
		{
			UnusedAssetsData.Add(SelectedAssetData);
		}
	}

	// 제거할 애셋이 없는 경우
	if (UnusedAssetsData.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No unused asset found among selected assets"), false);
		return;
	}

	// 컨테이너에 저장된 애셋들을 제거
	const int32 NumOfAssetsDeleted = ObjectTools::DeleteAssets(UnusedAssetsData);
	if (NumOfAssetsDeleted == 0)
	{
		return;
	}

	// 제거한 애셋의 개수를 출력
	DebugHeader::ShowNotifyInfo(TEXT("Successfully deleted " + FString::FromInt(NumOfAssetsDeleted) + TEXT(" unused assets")));
}

void UQuickAssetActions::FixUpRedirectors(const TArray<FName>& PackagePaths)
{
	// 엔진 내의 모든 에셋 정보를 관리하는 AssetRegistry 모듈을 불러옴
	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();

	// 애셋의 검색 조건인 ARFilter를 설정
	// 재귀적으로 탐색하며 인자로 받은 PackagePaths 아래의 ObjectRedirector 애셋을 탐색하도록 설정
	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Append(PackagePaths);
	Filter.ClassPaths.Add(UObjectRedirector::StaticClass()->GetClassPathName());

	// 설정한 ARFilter를 사용하여 애셋을 탐색
	TArray<FAssetData> AssetList;
	AssetRegistry.GetAssets(Filter, AssetList);

	if (AssetList.Num() == 0)
	{
		return;
	}

	// 탐색한 애셋의 Object Path 저장
	TArray<FString> ObjectPaths;
	for (const FAssetData& Asset : AssetList)
	{
		ObjectPaths.Add(Asset.GetObjectPathString());
	}

	// 불러온 애셋들을 저장할 TArray + 애셋 로드를 시도할 지 여부 + Redirector를 로드할지 여부
	TArray<UObject*> Objects;
	const bool bAllowedToPromptToLoadAssets = true;
	const bool bLoadRedirects = true;

	// 애셋을 로드하는 LoadAssetsSettings를 설정
	// bFollowRedirectors를 False로 설정하여 Redirector 객체 자체를 로드
	// bAllowCancel를 True로 설정하여 로드중에 취소할 수 있도록 설정
	AssetViewUtils::FLoadAssetsSettings Settings;
	Settings.bFollowRedirectors = false;
	Settings.bAllowCancel = true;

	// 탐색한 애셋의 Object Path + 불러온 애셋을 저장할 TArray + LoadAssetsSettings를 인자로 넘겨 LoadAssetsIfNeeded 함수 호출
	// 필요한 경우에만 애셋을 로드하는 함수이며, 대량의 에셋을 로드할 때 발생하는 부하를 관리
	AssetViewUtils::ELoadAssetsResult Result = AssetViewUtils::LoadAssetsIfNeeded(ObjectPaths, Objects, Settings);

	// 애셋을 로드하는 과정에서 취소하지 않았다면,
	if (Result != AssetViewUtils::ELoadAssetsResult::Cancelled)
	{
		// 로드한 오브젝트들을 순회하면서 ObjectRedirector로 캐스팅하여 저장
		TArray<UObjectRedirector*> Redirectors;
		for (UObject* Object : Objects)
		{
			Redirectors.Add(CastChecked<UObjectRedirector>(Object));
		}

		// 애셋 관련 유틸리티 기능을 제공하는 AssetTools 모듈을 불러옴 
		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));

		// AssetTools 모듈로부터 FixupReferencers 함수를 호출하여 애셋들에 대해 리디렉션
		AssetToolsModule.Get().FixupReferencers(Redirectors);
	}
}

TArray<FName> UQuickAssetActions::GetTopLevelPackagePath(const TArray<FAssetData>& Array)
{
	TSet<FName> TopLevelPaths;

	for (const FAssetData& AssetData : Array)
	{
		FString Path = AssetData.PackagePath.ToString();
		if (Path.IsEmpty())
		{
			continue;
		}

		int32 SecondSlashIdx = Path.Find(TEXT("/"), ESearchCase::IgnoreCase, ESearchDir::FromStart, 1);

		if (SecondSlashIdx != INDEX_NONE)
		{
			TopLevelPaths.Add(*Path.Left(SecondSlashIdx));
		}
		else
		{
			TopLevelPaths.Add(*Path);
		}
	}

	return TopLevelPaths.Array();
}