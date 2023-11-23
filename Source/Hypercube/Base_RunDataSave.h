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
	bool PlayerWon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Score;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EnemiesPercentageKilled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int TotalEnemies;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxMultiplicator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OnDeathMultiplicator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlayTime;

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
