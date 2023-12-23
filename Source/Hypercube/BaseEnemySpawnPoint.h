// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseEnemySpawnPoint.generated.h"

UCLASS()
class HYPERCUBE_API ABaseEnemySpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABaseEnemySpawnPoint();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EnemyParameters)
	class UClass* EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EnemyParameters)
	float EnemySpawnHeight;

	UFUNCTION(BlueprintCallable)
	class ABaseNPCSimpleChase* SpawnEnemy() const;

};
