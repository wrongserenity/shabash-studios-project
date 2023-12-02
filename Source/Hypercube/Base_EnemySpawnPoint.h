// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Base_EnemySpawnPoint.generated.h"

UCLASS()
class HYPERCUBE_API ABase_EnemySpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABase_EnemySpawnPoint();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EnemyParameters)
	class UClass* EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EnemyParameters)
	float EnemySpawnHeight;

	UFUNCTION(BlueprintCallable)
	class ABase_NPC_SimpleChase* SpawnEnemy() const;

};
