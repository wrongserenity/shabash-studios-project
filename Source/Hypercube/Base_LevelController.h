// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Base_RunDataSave.h"
#include "Base_LevelController.generated.h"

UCLASS()
class HYPERCUBE_API ABase_LevelController : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABase_LevelController();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SaveSlotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FLevelData> LevelData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLevelData CurLevelData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AfterPlayerDeathTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName NextLevelName;

protected:

	class AHypercubeCharacter* Player;
	
	TSet<class ABase_NPC_SimpleChase*> Enemies;

	FTimerHandle AfterPlayerDeathTimerHandle;

	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

public:	

	UFUNCTION(BlueprintCallable)
	void SetPlayerCharacter(class AHypercubeCharacter* PlayerCharacter);

	UFUNCTION(BlueprintCallable)
	void AddEnemy(class ABase_NPC_SimpleChase* EnemyCharacter);

	UFUNCTION(BlueprintCallable)
	void AddEnemiesKilled();

	UFUNCTION(BlueprintCallable)
	void RemoveEnemy(class ABase_NPC_SimpleChase* Enemy);

	UFUNCTION(BlueprintCallable)
	void UpdateMaxMultiplicator(float NewMultiplicator);

	UFUNCTION(BlueprintCallable)
	void OnPlayerDeath();

	UFUNCTION(BlueprintCallable)
	void AfterPlayerDeath();

	UFUNCTION(BlueprintCallable)
	void ClearLevelData();

};
