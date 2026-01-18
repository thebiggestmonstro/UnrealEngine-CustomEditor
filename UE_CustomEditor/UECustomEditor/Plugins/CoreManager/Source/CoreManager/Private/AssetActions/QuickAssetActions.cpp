// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetActions/QuickAssetActions.h"

void UQuickAssetActions::TestFunc()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Cyan, TEXT("Working"));
	}
}
