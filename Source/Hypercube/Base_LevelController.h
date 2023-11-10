// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Base_RunDataSave.h"
#include "Base_LevelController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllEnemiesDead);

UCLASS()
class HYPERCUBE_API ABase_LevelController : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABase_LevelController();

	UPROPERTY(BlueprintAssignable, Category = EventDispatchers)
	FOnAllEnemiesDead AllEnemiesDeadDelegate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SaveSlotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FLevelData> LevelData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLevelData CurLevelData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AfterPlayerDeathTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AfterAllEnemiesDeadTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName NextLevelName;

protected:

	class AHypercubeCharacter* Player;
	
	TSet<class ABase_NPC_SimpleChase*> Enemies;

	FTimerHandle AfterLevelTimerHandle;

	virtual void BeginPlay() override;
	//virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

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
	void OnAllEnemiesDead();

	UFUNCTION(BlueprintCallable)
	void AfterAllEnemiesDead();

	UFUNCTION(BlueprintCallable)
	void SaveLevelData();

	UFUNCTION(BlueprintCallable)
	void LoadNewLevel();

	UFUNCTION(BlueprintCallable)
	void ClearLevelData();

	UFUNCTION(BlueprintCallable)
	float GetPlayerHealthValue();

};
