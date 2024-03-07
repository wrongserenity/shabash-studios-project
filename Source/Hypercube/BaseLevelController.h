#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseRunDataSave.h"
#include "BaseNPCSimpleChase.h"
#include "BaseLevelController.generated.h"

// Base class for level controller
// Level controller provides data saving and loading, enemy spawning, difficulty settings realisation and dynamic music

USTRUCT(BlueprintType)
struct FScoreboardData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Score;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DifficultyParameter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int LevelIndex;

	bool operator<(const FScoreboardData& Other) const;
};

USTRUCT(BlueprintType)
struct FChainBoostVFXData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class ABaseNPCSimpleChase* Enemy1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class ABaseNPCSimpleChase* Enemy2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UNiagaraComponent* DefaultVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UNiagaraComponent* DamageVFX;
};

UENUM(BlueprintType)
enum class EEnemyStackState : uint8
{
	None UMETA(DisplayName = "None"),
	SoftStack UMETA(DisplayName = "SoftStack"),
	HardStack UMETA(DisplayName = "HardStack")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllEnemiesDead);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFewEnemiesRemaining);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHardStackDeactivation);

UCLASS()
class HYPERCUBE_API ABaseLevelController : public AActor
{
	GENERATED_BODY()

	// Root component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* Root;

	// Music component which plays exploration music
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class UAudioComponent* MusicCompExplore;

	// Music component which plays low intensity battle music
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class UAudioComponent* MusicCompLow;

	// Music component which plays high intensity battle music layer
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class UAudioComponent* MusicCompHigh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Particles, meta = (AllowPrivateAccess = "true"))
	class UNiagaraComponent* ChainBoostNiagaraAsset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Particles, meta = (AllowPrivateAccess = "true"))
	class UNiagaraComponent* ChainBoostDamageNiagaraAsset;

public:

	ABaseLevelController();

	UPROPERTY(BlueprintAssignable, Category = EventDispatchers)
	FOnAllEnemiesDead AllEnemiesDeadDelegate;

	// Broadcasting when few enemies remaining which tells remaining enemies to chase player even if they did not notice them
	UPROPERTY(BlueprintAssignable, Category = EventDispatchers)
	FOnFewEnemiesRemaining FewEnemiesRemainingDelegate;

	UPROPERTY(BlueprintAssignable, Category = EventDispatchers)
	FOnHardStackDeactivation HardStackDeactivationDelegate;

protected:

	// Name of save slot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SaveGame, meta = (AllowPrivateAccess = "true"));
	FString SaveSlotName;

	// Array of data loaded from save file
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SaveGame, meta = (AllowPrivateAccess = "true"))
	TArray<FLevelData> LevelData;

	// Data of current walkthrough that will be saved on level transition
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SaveGame, meta = (AllowPrivateAccess = "true"))
	FLevelData CurLevelData;

	// Time between player death and level restart
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats, meta = (AllowPrivateAccess = "true"))
	float AfterPlayerDeathTime;

	// If enemy percentage drops below this, FewEnemiesRemainingDelegate will be broadcasted
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats, meta = (AllowPrivateAccess = "true"))
	float FewEnemiesEventPercentage;

	// Defines enemy stats increasing 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Stats, meta = (AllowPrivateAccess = "true"))
	float EnemyLevelingPercentage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Stats, meta = (AllowPrivateAccess = "true"))
	EEnemyStackState StackState;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Stats, meta = (AllowPrivateAccess = "true"))
	int SoftStackBeginEnemyCount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Stats, meta = (AllowPrivateAccess = "true"))
	int SoftStackEndEnemyCount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Stats, meta = (AllowPrivateAccess = "true"))
	int HardStackBeginEnemyCount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Stats, meta = (AllowPrivateAccess = "true"))
	int HardStackEndEnemyCount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Stats, meta = (AllowPrivateAccess = "true"))
	float EnemyStackQueryFrequency;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Stats, meta = (AllowPrivateAccess = "true"))
	float ChainDamageMultiplier;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Stats, meta = (AllowPrivateAccess = "true"))
	TArray<FString> LevelNames;

	// Level names that are shown in scoreboard
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Stats, meta = (AllowPrivateAccess = "true"))
	TArray<FString> LevelNamesToShow;

	// How frequent can enemies make notice sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Sound", meta = (AllowPrivateAccess = "true"))
	float NoticeSoundTurnOffTime;

	// If enemy can make notice sound
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats | Sound", meta = (AllowPrivateAccess = "true"))
	bool bEnemyCanNoticeSound;

	// How frequent can enemies make footstep sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Sound", meta = (AllowPrivateAccess = "true"))
	float FootstepSoundTurnOffTime;

	// If enemy can make footstep sound
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats | Sound", meta = (AllowPrivateAccess = "true"))
	bool bEnemyCanFootstepSound;

	// How frequent can enemies make death sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Sound", meta = (AllowPrivateAccess = "true"))
	float DeathSoundTurnOffTime;

	// If enemy can make death sound
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats | Sound", meta = (AllowPrivateAccess = "true"))
	bool bEnemyCanDeathSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive difficulty | Input data | Death count", meta = (AllowPrivateAccess = "true"))
	TArray<int> DeathCountBounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive difficulty | Input data | Death count", meta = (AllowPrivateAccess = "true"))
	TArray<float> DeathCountValues;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive difficulty | Input data | Death count", meta = (AllowPrivateAccess = "true"))
	float DeathCountCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive difficulty | Input data | Enemy aggro on death", meta = (AllowPrivateAccess = "true"))
	TArray<int> OnDeathEnemyAggroBounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive difficulty | Input data | Enemy aggro on death", meta = (AllowPrivateAccess = "true"))
	TArray<float> OnDeathEnemyAggroValues;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive difficulty | Input data | Enemy aggro on death", meta = (AllowPrivateAccess = "true"))
	float OnDeathEnemyAggroCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive difficulty | Input data | Play time", meta = (AllowPrivateAccess = "true"))
	TArray<float> PlayTimeBounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive difficulty | Input data | Play time", meta = (AllowPrivateAccess = "true"))
	TArray<float> PlayTimeValues;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive difficulty | Input data | Play time", meta = (AllowPrivateAccess = "true"))
	float PlayTimeCost;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Adaptive difficulty | Output parameters", meta = (AllowPrivateAccess = "true"))
	float DifficultyParameter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive difficulty | Output parameters", meta = (AllowPrivateAccess = "true"))
	TArray<float> DifficultyParameterBounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive difficulty | Output parameters | Player", meta = (AllowPrivateAccess = "true"))
	TArray<float> PlayerVelocityValues;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive difficulty | Output parameters | Player", meta = (AllowPrivateAccess = "true"))
	TArray<float> PlayerDamageMultiplerValues;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive difficulty | Output parameters | Player", meta = (AllowPrivateAccess = "true"))
	TArray<float> PlayerVampirismValues;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive difficulty | Output parameters | Enemies", meta = (AllowPrivateAccess = "true"))
	TArray<float> EnemyVelocityValues;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive difficulty | Output parameters | Enemies", meta = (AllowPrivateAccess = "true"))
	TArray<float> EnemyDamageValues;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive difficulty | Output parameters | Enemies", meta = (AllowPrivateAccess = "true"))
	TArray<float> EnemyNoticeRadiusValues;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive difficulty | Output parameters | Enemies", meta = (AllowPrivateAccess = "true"))
	TArray<float> EnemyCountPercentageValues;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive difficulty | Output parameters | Enemies", meta = (AllowPrivateAccess = "true"))
	TArray<float> EnemyLevelingPercentageValues;

	// Value in [0.0, 1.0], where 0.0 - no battle (exploration music), 0.5 - low intensity battle, 1.0 - high intensity battle
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Music)
	float MusicParameter;

	// How fast is music blending
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Music)
	float MusicChangeSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Music)
	float TargetMusicParameter;

	// How frequent music parameter is recalculated
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Music)
	float MusicRefreshFrequency;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Music)
	float MusicVolumeMultiplier;

protected:

	// Index of current level in LevelNames
	int CurLevelIndex;

	class AHypercubeCharacter* Player;

	TArray<class AActor*> SpawnPoints;

	int BeginEnemyCount;
	int EnemiesKilled;
	int FewEnemiesEventCount;
	TSet<class ABaseNPCSimpleChase*> Enemies;

	FTimerHandle AfterLevelTimerHandle;

	float MusicRefreshTimer;

	TArray<FScoreboardData> Scores;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	int GetCurMapIndex() const;

	bool bIsHardStackActive;
	FTimerHandle EnemyStackQueryTimerHandle;
	void EnemyStackQuery();

	TSet<class ABaseNPCSimpleChase*> ChainedEnemies;
	TArray<FChainBoostVFXData> ChainVFXData;

	void ReadScoreboardData();

	FTimerHandle NoticeSoundTurnOffTimerHandle;
	void OnEndNoticeSoundTurnedOff();

	FTimerHandle FootstepSoundTurnOffTimerHandle;
	void OnEndFootstepSoundTurnedOff();

	FTimerHandle DeathSoundTurnOffTimerHandle;
	void OnEndDeathSoundTurnedOff();

public:

	// Load data from save file
	UFUNCTION(BlueprintCallable)
	void LoadLevelData();

	// Spawn enemies on spawn points
	UFUNCTION(BlueprintCallable)
	void SpawnEnemies();

	UFUNCTION(BlueprintCallable)
	void SpawnEnemy(class ABaseEnemySpawnPoint* SpawnPoint, int Level = 0, EEnemyLevelingType LevelingType = EEnemyLevelingType::None);

	UFUNCTION(BlueprintCallable)
	void SetPlayerCharacter(class AHypercubeCharacter* PlayerCharacter);

	// Add enemy to enemy set
	UFUNCTION(BlueprintCallable)
	void AddEnemy(class ABaseNPCSimpleChase* EnemyCharacter);

	// Called on enemy death
	UFUNCTION(BlueprintCallable)
	void RemoveEnemy(class ABaseNPCSimpleChase* Enemy);

	// Called on enemy death
	UFUNCTION(BlueprintCallable)
	void UpdateMaxMultiplicator(float NewMultiplicator);

	UFUNCTION(BlueprintCallable)
	void OnPlayerDeath();

	// Called AfterPlayerDeathTime seconds after OnPlayerDeath()
	UFUNCTION(BlueprintCallable)
	void AfterPlayerDeath();

	UFUNCTION(BlueprintCallable)
	void OnAllEnemiesDead();

	// Saves current level data in save file
	UFUNCTION(BlueprintCallable)
	void SaveLevelData();

	// Clears save file
	UFUNCTION(BlueprintCallable)
	void ClearLevelData();

	// Restart current level
	UFUNCTION(BlueprintCallable)
	void ReloadCurrentLevel();

	UFUNCTION(BlueprintCallable)
	void LoadNextLevel();

	UFUNCTION(BlueprintCallable)
	void SetNoticeSoundTurnOff();

	UFUNCTION(BlueprintCallable)
	void SetFootstepSoundTurnOff();

	UFUNCTION(BlueprintCallable)
	void SetDeathSoundTurnOff();

	// Calculate difficulty parameter
	UFUNCTION(BlueprintCallable)
	float GetDifficultyParameter();

	UFUNCTION(BlueprintCallable)
	void SetPlayerParams();

	UFUNCTION(BlueprintCallable)
	void SetEnemyParams(class ABaseNPCSimpleChase* Enemy);

	UFUNCTION(BlueprintCallable)
	float GetTargetMusicParameter() const;

	// Called upon enemy chasing count change
	UFUNCTION(BlueprintCallable)
	void UpdateStackState();

	UFUNCTION(BlueprintCallable)
	void SoftStack();

	UFUNCTION(BlueprintCallable)
	void HardStack();

	UFUNCTION(BlueprintCallable)
	void StackEnemies(class ABaseNPCSimpleChase* Enemy1, class ABaseNPCSimpleChase* Enemy2);

	UFUNCTION(BlueprintCallable)
	void AddEnemyToChain(class ABaseNPCSimpleChase* Enemy);

	UFUNCTION(BlueprintCallable)
	void RemoveEnemyFromChain(class ABaseNPCSimpleChase* Enemy);

	UFUNCTION(BlueprintCallable)
	void DealChainDamageExcept(class ABaseNPCSimpleChase* Enemy, float Damage);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FString GetScoreboard(int Num);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FString GetScoreboardEnumerate(int Num);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FString GetScoreboardLevels(int Num);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FString GetScoreboardScores(int Num);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FString GetScoreboardDiffs(int Num);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FString GetDifficultyBrief() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetEnemyLevelingPercentage() { return EnemyLevelingPercentage; }
};

// Returns 1 when array is increasing, -1 when decreasing, else 0
template<typename T>
int GetAsc(const TArray<T>& Arr)
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

// Returns unscaled difficulty parameter from given bounds and parameter values arrays
template<typename T>
float GetDifficultyParameterFrom(T Val, const TArray<T>& Bounds, const TArray<float>& Values)
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

// Returns game parameter from given difficulty parameter, bounds and values arrays
template<typename T>
T GetOutputParameterFrom(float Val, const TArray<float>& Bounds, const TArray<T>& Values)
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

int GetEnemyIndexWithMinDistance(const TArray<class ABaseNPCSimpleChase*>& Enemies, class ABaseNPCSimpleChase* EnemyToCompare);

FString FloatToFString(float Val);