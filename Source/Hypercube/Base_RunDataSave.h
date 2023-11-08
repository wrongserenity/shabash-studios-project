// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Base_RunDataSave.generated.h"

USTRUCT(BlueprintType)
struct FLevelData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Score;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int EnemiesKilled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxMultiplicator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OnDeathMultiplicator;

	void Log();
};

UCLASS()
class HYPERCUBE_API UBase_RunDataSave : public USaveGame
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere)
	TArray<FLevelData> LevelDataArr;

};
