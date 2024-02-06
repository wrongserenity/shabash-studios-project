// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "BaseRunDataSave.generated.h"

USTRUCT(BlueprintType)
struct FLevelData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPlayerWon;

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
	int OnDeathEnemyChasing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlayTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DifficultyParameter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int LevelIndex;

	void Log();
};

UCLASS()
class HYPERCUBE_API UBaseRunDataSave : public USaveGame
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere)
	TArray<FLevelData> LevelDataArr;

};
