// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Base_RunDataSave.h"
#include "Containers/SortedMap.h"
#include "Base_LevelController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllEnemiesDead);

UCLASS()
class HYPERCUBE_API ABase_LevelController : public AActor
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UAudioComponent* MusicComp_Low;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UAudioComponent* MusicComp_High;

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
	float FootstepSoundTurnOffTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Stats)
	bool bEnemyCanFootstepSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats)
	float DeathSoundTurnOffTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Stats)
	bool bEnemyCanDeathSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats)
	TArray<FString> LevelNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive difficulty | Input data | Death count")
	TArray<int> DeathCountBounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive difficulty | Input data | Death count")
	TArray<float> DeathCountValues;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive difficulty | Input data | Death count")
	float DeathCountCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive difficulty | Input data | Enemy aggro on death")
	TArray<int> OnDeathEnemyAggroBounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive difficulty | Input data | Enemy aggro on death")
	TArray<float> OnDeathEnemyAggroValues;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive difficulty | Input data | Enemy aggro on death")
	float OnDeathEnemyAggroCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive difficulty | Input data | Play time")
	TArray<float> PlayTimeBounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive difficulty | Input data | Play time")
	TArray<float> PlayTimeValues;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive difficulty | Input data | Play time")
	float PlayTimeCost;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Adaptive difficulty | Output parameters")
	float DifficultyParameter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive difficulty | Output parameters")
	TArray<float> DifficultyParameterBounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive difficulty | Output parameters | Player")
	TArray<float> PlayerVelocityValues;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive difficulty | Output parameters | Player")
	TArray<float> PlayerDamageMultiplerValues;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive difficulty | Output parameters | Player")
	TArray<float> PlayerVampirismValues;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive difficulty | Output parameters | Enemies")
	TArray<float> EnemyVelocityValues;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive difficulty | Output parameters | Enemies")
	TArray<float> EnemyDamageValues;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive difficulty | Output parameters | Enemies")
	TArray<float> EnemyNoticeRadiusValues;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive difficulty | Output parameters | Enemies")
	TArray<float> EnemyCountPercentageValues;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Music")
	float MusicParameter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	float MusicChangeSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	float TargetMusicParameter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	float MusicRefreshFrequency;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
	float MusicVolumeMultiplier;

protected:

	int CurLevelIndex;

	class AHypercubeCharacter* Player;
	
	TArray<class AActor*> SpawnPoints;

	int BeginEnemyCount;
	int EnemiesKilled;
	TSet<class ABase_NPC_SimpleChase*> Enemies;

	FTimerHandle AfterLevelTimerHandle;

	float MusicRefreshTimer;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	//virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	int GetCurMapIndex() const;

	FTimerHandle NoticeSoundTurnOffTimerHandle;
	void OnEndNoticeSoundTurnedOff();

	FTimerHandle FootstepSoundTurnOffTimerHandle;
	void OnEndFootstepSoundTurnedOff();

	FTimerHandle DeathSoundTurnOffTimerHandle;
	void OnEndDeathSoundTurnedOff();

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
	void ClearLevelData();

	UFUNCTION(BlueprintCallable)
	void SetNoticeSoundTurnOff();

	UFUNCTION(BlueprintCallable)
	void SetFootstepSoundTurnOff();

	UFUNCTION(BlueprintCallable)
	void SetDeathSoundTurnOff();

	UFUNCTION(BlueprintCallable)
	float GetDifficultyParameter();

	UFUNCTION(BlueprintCallable)
	void SetPlayerParams();

	UFUNCTION(BlueprintCallable)
	void SetEnemyParams(class ABase_NPC_SimpleChase* Enemy);

	UFUNCTION(BlueprintCallable)
	float GetTargetMusicParameter();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FString GetScoreboard(int Num) const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FString GetDifficultyBrief() const;
};

template<typename T>
int GetAsc(const TArray<T>& Arr) // returns 1 when array is increasing, -1 when decreasing, else 0
{
	if (Arr.Num() < 2)
	{
		return 1;
	}
	bool Asc = Arr[0] < Arr.Last();
	if (Asc)
	{
		for (int i = 0; i < Arr.Num() - 1; ++i)
		{
			if (Arr[i] > Arr[i + 1])
			{
				return 0;
			}
		}
	}
	else
	{
		for (int i = 0; i < Arr.Num() - 1; ++i)
		{
			if (Arr[i] < Arr[i + 1])
			{
				return 0;
			}
		}
	}
	return Asc ? 1 : -1;
}

template<typename T>
float GetDifficultyParameterFrom(T Val, const TArray<T>& Bounds, const TArray<float>& Values) // returns unscaled difficulty parameter from given bounds and parameter values arrays
{
	if (Bounds.Num() != Values.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("Adaptive difficulty: Bounds array must be the same length as values array!"));
		return 0.5f;
	}
	if (!GetAsc(Bounds) || !GetAsc(Values))
	{
		UE_LOG(LogTemp, Error, TEXT("Adaptive difficulty: Bounds and values arrays must be monotone"));
		return 0.5f;
	}
	bool BoundsAsc = Bounds[0] < Bounds.Last();
	bool ValuesAsc = Values[0] < Values.Last();
	for (int i = 0; i < Bounds.Num(); ++i)
	{
		if ((BoundsAsc && Val < Bounds[i]) || (!BoundsAsc && Val > Bounds[i]))
		{
			return Values[i];
		}
	}
	return ValuesAsc ? 1.0f : 0.0f;
}

template<typename T>
T GetOutputParameterFrom(float Val, const TArray<float>& Bounds, const TArray<T>& Values) // returns game parameter from given difficulty parameter, bounds and values arrays
{
	if (Bounds.Num() != Values.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("Adaptive difficulty: Bounds array must be the same length as values array!"));
		return T();
	}
	if (GetAsc(Bounds) != 1)
	{
		UE_LOG(LogTemp, Error, TEXT("Adaptive difficulty: Bounds array must be monotone"));
		return T();
	}
	for (int i = 1; i < Bounds.Num(); ++i)
	{
		if (Val < Bounds[i])
		{
			return Values[i - 1];
		}
	}
	return Values.Last();
}

FString FloatToFString(float Val);