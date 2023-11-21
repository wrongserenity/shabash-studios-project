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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SaveGame)
	FString SaveSlotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SaveGame)
	TArray<FLevelData> LevelData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SaveGame)
	FLevelData CurLevelData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats)
	float AfterPlayerDeathTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats)
	float AfterAllEnemiesDeadTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats)
	float NoticeSoundTurnOffTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Stats)
	bool bEnemyCanNoticeSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats)
	FName NextLevelName;

protected:

	class AHypercubeCharacter* Player;
	
	TArray<class AActor*> SpawnPoints;

	int BeginEnemyCount;
	int EnemiesKilled;
	TSet<class ABase_NPC_SimpleChase*> Enemies;

	FTimerHandle AfterLevelTimerHandle;
	FTimerHandle NoticeSoundTurnOffTimerHandle;

	virtual void BeginPlay() override;
	//virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

public:	

	UFUNCTION(BlueprintCallable)
	void LoadLevelData();

	UFUNCTION(BlueprintCallable)
	void SpawnEnemies();

	UFUNCTION(BlueprintCallable)
	void SpawnEnemy(class ABase_EnemySpawnPoint* SpawnPoint);

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

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetPlayerHealthValue() const;

	UFUNCTION(BlueprintCallable)
	float GetEnemyPercentage() const;

	UFUNCTION(BlueprintCallable)
	void SetNoticeSoundTurnOff();

	UFUNCTION(BlueprintCallable)
	void OnEndNoticeSoundTurnedOff();

};
