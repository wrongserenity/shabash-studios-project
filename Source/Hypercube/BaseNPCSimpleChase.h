#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseNPCSimpleChase.generated.h"

// Base class for enemy NPC

USTRUCT(BlueprintType)
struct FAttackStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OpenerTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AfterAttackTime;

	// Speed with which enemy are rotating towards the player while attacking
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackRotationMultiplier;

	// Speed with which enemy are going forward during active frames of attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackMoveForwardSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackWidth;
};

UENUM(BlueprintType)
enum class EAttackPhase : uint8
{
	NotAttacking UMETA(DisplayName = "NotAttacking"),
	Opener UMETA(DisplayName = "Opener"),
	Attacking UMETA(DisplayName = "Attacking"),
	AfterAttack UMETA(DisplayName = "AfterAttack")
};

UENUM(BlueprintType)
enum class EEnemyPhase : uint8
{
	None UMETA(DisplayName = "None"),
	Noticing UMETA(DisplayName = "Noticing"),
	Chasing UMETA(DisplayName = "Chasing")
};

UENUM(BlueprintType)
enum class EEnemyAction : uint8
{
	AttackEnd UMETA(DisplayName = "AttackEnd"),
	JumpEnd UMETA(DisplayName = "JumpEnd"),
	UnstuckEnd UMETA(DisplayName = "UnstuckEnd"),
	Damaged UMETA(DisplayName = "Damaged"),
	SlowDebuff UMETA(DisplayName = "SlowDebuff"),
	SlowDebuffEnd UMETA(DisplayName = "SlowDebuffEnd"),
	DamageDecreaseDebuff UMETA(DisplayName = "AttackDecreaseDebuff"),
	DamageDecreaseDebuffEnd UMETA(DisplayName = "AttackDecreaseDebuffEnd"),
	HealBuff UMETA(DisplayName = "HealBuff"),
	HealBurst UMETA(DisplayName = "HealBurst"),
	HealBuffEnd UMETA(DisplayName = "HealBuffEnd"),
	LevelUpdate UMETA(DisplayName = "LevelUpdate")
};

UENUM(BlueprintType)
enum class EEnemyLevelingType : uint8
{
	None UMETA(DisplayName = "None"),
	Speed UMETA(DisplayName = "Speed"),
	Damage UMETA(DisplayName = "Damage"),
	Health UMETA(DisplayName = "Health")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyAction, EEnemyAction, Action, bool, Success);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyStackQuery, ABaseNPCSimpleChase*, OtherEnemy);

UCLASS()
class HYPERCUBE_API ABaseNPCSimpleChase : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Component, meta = (AllowPrivateAccess = "true"))
	class UCapsuleComponent* Capsule;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class UCharacterMovementComponent* MoveComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* NoticeCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* AttackCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* DebugAttackCollision;

	// Some mesh appearing above character when damaged
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* DebugDamageIndicator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Particles, meta = (AllowPrivateAccess = "true"))
	class UParticleSystemComponent* HealBuffParticleSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Particles, meta = (AllowPrivateAccess = "true"))
	class UParticleSystemComponent* StackParticleSystem;

public:

	ABaseNPCSimpleChase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = LevelController)
	class ABaseLevelController* LevelController;

	UPROPERTY(BlueprintAssignable, Category = EventDispatchers)
	FOnEnemyDeath EnemyDeathDelegate;

	UPROPERTY(BlueprintAssignable, Category = EventDispatchers)
	FOnEnemyAction EnemyActionDelegate;

	UPROPERTY(BlueprintAssignable, Category = EventDispatchers)
	FOnEnemyStackQuery EnemyStackQueryDelegate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Health")
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Health")
	float MaxHealth;

	// Time between heal bursts while healing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Health")
	float HealBurstTimeBetween;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Jump")
	float JumpTime;

	// Radius of visibility
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Aggro")
	float AggroRadius;

	// Time between aggro and start of chasing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Aggro")
	float AggroTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Attack")
	FAttackStats SimpleAttack;

	// Number of seconds between checks of player sight on enemy when enemy is stuck
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Unstuck")
	float UnstuckPlayerSightUpdate;

	// Radius around player in which stuck enemies will be teleported
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Unstuck")
	float UnstuckAroundPlayerRadius;

	// Maximum number of attemps to unstuck during one iteration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Unstuck")
	int MaxAttempsToUnstuck;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | VFX")
	float StackVFXTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bIsDebugOn;

	// Time for which debug damage indicator becames visible
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	float DebugDamageIndicatorTime;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats | Leveling", meta = (AllowPrivateAccess = "true"))
	int Level;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats | Leveling", meta = (AllowPrivateAccess = "true"))
	EEnemyLevelingType LevelingType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats | Effects", meta = (AllowPrivateAccess = "true"))
	bool bIsChained;

protected:

	// Semaphore that controls Tick() usage
	uint8 TickSemaphore;

	FTimerHandle NoticeTimerHandle;
	EEnemyPhase MovePhase;

	FTimerHandle AttackTimerHandle;
	EAttackPhase AttackPhase;
	class AHypercubeCharacter* Player;

	// Dealayed initialization
	FTimerHandle DelayedInitTimerHandle;
	float DelayedInitTime;
	void DelayedInit();

	// Pass true when Tick() is needed (increment the semaphore), false when it becames unnecessary (decrement the semaphore)
	void SetTickState(bool bToActivate);

	// Force disable of ticking and setting TickSemaphore = 0
	void ForceTickDisable();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void TickRotateToTarget(float DeltaSeconds);
	void TickMoveForward(float DeltaSeconds);
	void CheckPlayerHit();

	void AfterNotice();
	
	void SetAttackCollision(bool bToActivate);
	void SetDebugAttackCollision(bool bToActivate);

	FTimerHandle DebugDamageIndicatorTimerHandle;
	void ActivateDebugDamageIndicator();
	void OnEndDebugDamageIndicatorTimer();

	FTimerHandle JumpTimerHandle;
	void OnEndJump();

	FTimerHandle SlowDebuffTimerHandle;
	float BaseSpeed;
	void OnEndSlowDebuff();

	FTimerHandle DamageDebuffTimerHandle;
	float BaseDamage;
	void OnEndDamageDebuff();

	float HealPerBurst;
	float HealRemaining;
	bool bIsHealing;
	FTimerHandle HealBuffTimerHandle;
	void HealBurst();
	void OnEndHealBuff();

	FTimerHandle StackVFXTimerHandle;
	void OnEndStackVFX();

	FTimerHandle ChainBoostTimerHandle;
	void OnEndChainBoost();

	void ResetLevel();


	FTimerHandle CheckPlayerSightTimerHandle;

public:

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE class ABaseLevelController* GetLevelController() const { return LevelController; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE class USphereComponent* GetNoticeCollision() const { return NoticeCollision; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE int GetEnemyLevel() const { return Level; }

	UFUNCTION(BlueprintCallable)
	void TakeDamage(float Damage, bool bIgnoreChain = false);

	UFUNCTION(BlueprintCallable)
	void OnNotice();

	// Execute simple attack
	UFUNCTION(BlueprintCallable)
	void Attack();

	UFUNCTION(BlueprintCallable)
	void JumpTo(FVector Destination);

	// Called when health drops below zero
	UFUNCTION(BlueprintCallable)
	void PlayDeath();

	UFUNCTION(BlueprintCallable)
	void SetSlowDebuff(float Mult, float Time);

	UFUNCTION(BlueprintCallable)
	void SetDamageDebuff(float Mult, float Time);

	UFUNCTION(BlueprintCallable)
	void SetHealBuff(float Heal, int BurstCount);

	// True if player camera has sight on this NPC
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool PlayerHasSightOn() const;

	// Called when NPC is stuck and player is not seeing it. Teleports NPC near plauyer out of his sight
	UFUNCTION(BlueprintCallable)
	void Unstuck();

	// Get stat multiplier according to enemy level
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetStatMultiplier() const;

	// How does Damage multiplier increase multiplies according to enemy level
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetDamageMultiplierMultiplier() const;

	UFUNCTION(BlueprintCallable)
	void SetLevel(int NewLevel, EEnemyLevelingType NewLevelingType);

	UFUNCTION(BlueprintCallable)
	void IncreaseLevel(int ToIncrease);

	UFUNCTION(BlueprintCallable)
	void StackWith(ABaseNPCSimpleChase* OtherEnemy);

	UFUNCTION(BlueprintCallable)
	void PlayStackVFX();

	UFUNCTION(BlueprintCallable)
	void SetChainBoost(float Time);
};
