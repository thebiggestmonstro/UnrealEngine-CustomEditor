// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetActions/QuickAssetActions.h"
#include "DebugHeader.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"

void UQuickAssetActions::DuplicateAssets(int32 NumOfDuplicates)
{
	if (NumOfDuplicates <= 0)
	{
		ShowMsgDialog(EAppMsgType::Ok, TEXT("Please enter a VALID number"));
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
		ShowNotifyInfo(TEXT("Successfully duplicated " + FString::FromInt(Counter) + " files"));
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

		// 탐색한 애셋 타입 + 타입별 접두사가 존재하지않는 경우
		if (!PrefixFound || PrefixFound->IsEmpty())
		{
			Print(TEXT("Failed to find prefix for class ") + SelectedObject->GetClass()->GetName(), FColor::Red);
			continue;
		}

		// 선택한 애셋이 이미 타입별 접두사로 이름 붙여진 경우
		if (OldName.StartsWith(*PrefixFound))
		{
			Print(OldName + TEXT(" already has prefix added"), FColor::Red);
			continue;
		}

		// Material Instance 애셋의 경우 원본 머티리얼의 접두사를 이름에서 제거
		if (SelectedObject->IsA<UMaterialInstanceConstant>())
		{
			OldName.RemoveFromStart(TEXT("M_"));
			OldName.RemoveFromEnd(TEXT("_Inst"));
		}

		// 선택한 애셋에 타입별 접두사를 수식
		const FString NewNameWithPrefix = *PrefixFound + OldName;
		UEditorUtilityLibrary::RenameAsset(SelectedObject, NewNameWithPrefix);

		// 선택한 애셋의 이름이 완전히 수정되었는지 확인
		if (SelectedObject->GetName() == NewNameWithPrefix)
		{
			++Counter;
		}
	}

	if (Counter > 0)
	{
		ShowNotifyInfo(TEXT("Successfully renamed " + FString::FromInt(Counter) + " assets"));
	}
}