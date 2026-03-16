// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "QuickActorActionsWidget.generated.h"

UENUM(BlueprintType)
enum class E_DuplicationAxis : uint8
{
	EDA_XAxis UMETA(DisplayName = "X Axis"),
	EDA_YAxis UMETA(DisplayName = "Y Axis"),
	EDA_ZAxis UMETA(DisplayName = "Z Axis"),
	EDA_MAX UMETA(DisplayName = "Default Max")
};

USTRUCT(BlueprintType)
struct FRandomActorRotation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomActorRotation")
	bool bRandomizeRotYaw = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bRandomizeRotYaw"), Category = "RandomActorRotation")
	float RotYawMin = -45.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bRandomizeRotYaw"), Category = "RandomActorRotation")
	float RotYawMax = 45.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomActorRotation")
	bool bRandomizeRotPitch = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bRandomizeRotPitch"), Category = "RandomActorRotation")
	float RotPitchMin = -45.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bRandomizeRotPitch", Category = "RandomActorRotation"))
	float RotPitchMax = 45.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomActorRotation")
	bool bRandomizeRotRoll = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bRandomizeRotRoll"), Category = "RandomActorRotation")
	float RotRollMin = -45.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bRandomizeRotRoll"), Category = "RandomActorRotation")
	float RotRollMax = 45.f;

};

/**
 * 
 */
UCLASS()
class COREMANAGER_API UQuickActorActionsWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "ActorBatchSelection")
	void SelectAllActorsWithSimilarName();

	UFUNCTION(BlueprintCallable, Category = "ActorBatchSelection")
	void SelectActorsWithName(const FString& ActorName);

	UFUNCTION(BlueprintCallable, Category = "ActorBatchSelection")
	void DeselectActorsWithName(const FString& ActorName);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorBatchSelection")
	TEnumAsByte<ESearchCase::Type> SearchCase = ESearchCase::IgnoreCase;

	// Actor Batch Duplication
	UFUNCTION(BlueprintCallable, Category = "ActorBatchDuplication")
	void DuplicateActors();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorBatchDuplication")
	E_DuplicationAxis AxisForDuplication = E_DuplicationAxis::EDA_XAxis;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorBatchDuplication")
	int32 NumberOfDuplicates = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorBatchDuplication")
	float OffsetDist = 300.f;

	// Randomize Actor Rotation
	UFUNCTION(BlueprintCallable, Category = "RandomizeActorTransform")
	void RandomizeActorTransform();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomizeActorTransform")
	FRandomActorRotation RandomActorRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomizeActorTransform")
	bool bRandomizeScale = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomizeActorTransform", meta = (EditCondition = "bRandomizeScale"))
	float ScaleMin = .8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomizeActorTransform", meta = (EditCondition = "bRandomizeScale"))
	float ScaleMax = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomizeActorTransform")
	bool bRandomizeOffset = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomizeActorTransform", meta = (EditCondition = "bRandomizeOffset"))
	float OffsetMin = -50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomizeActorTransform", meta = (EditCondition = "bRandomizeOffset"))
	float OffsetMax = 50.f;

private:
	UPROPERTY()
	class UEditorActorSubsystem* EditorActorSubsystem;

	bool GetEditorActorSubsystem();
};
