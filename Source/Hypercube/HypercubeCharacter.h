// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HypercubeCharacter.generated.h"

// Base class for player Character

UENUM(BlueprintType)
enum class EPlayerMovementPhase : uint8
{
	None UMETA(DisplayName = "None"),
	Walking UMETA(DisplayName = "Walking"),
	Dashing UMETA(DisplayName = "Dashing"),
	Attacking UMETA(DisplayName = "Attacking")
};

UENUM(BlueprintType)
enum class EPlayerAttackPhase : uint8
{
	None UMETA(DisplayName = "None"),
	Opener UMETA(DisplayName = "AttackOpener"),
	Attacking UMETA(DisplayName = "Attacking"),
	AfterAttack UMETA(DisplayName = "AfterAttack")
};

UENUM(BlueprintType)
enum class EPlayerAction : uint8
{
	Attack UMETA(DisplayName = "Attack"),
	Dash UMETA(DisplayName = "Dash"),
	Damaged UMETA(DisplayName = "Damaged"),
	HealBuff UMETA(DisplayName = "HealBuff"),
	HealBurst UMETA(DisplayName = "HealBurst"),
	HealBuffEnd UMETA(DisplayName = "HealBuffEnd")
};

USTRUCT(BlueprintType)
struct FPlayerAttackStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OpenerTime;

	// Time when attack hitbox is active
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AfterAttackTime;

	// Radius of attack hitbox
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackRadius;

	// Angle of attack swing
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackAngle;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerAction, EPlayerAction, Action);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPause, bool, bIsPaused);

UCLASS(config = Game)
class AHypercubeCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	// Capsule hitbox
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Collision, meta = (AllowPrivateAccess = "true"))
	class UCapsuleComponent* Capsule;

	// Box collision of attack
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Collision, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* AttackCollision;

	// Box component for attack phases debug viewing
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Debug, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* DebugAttackCollision;

	// Some mesh appearing above character when damaged
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Debug, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* DebugDamageIndicator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Controller, meta = (AllowPrivateAccess = "true"))
	class APlayerController* PlayerController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Particles, meta = (AllowPrivateAccess = "true"))
	class UParticleSystemComponent* HealBuffParticleSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Particles, meta = (AllowPrivateAccess = "true"))
	class UNiagaraComponent* SpeedBuffNiagara;

public:
	AHypercubeCharacter();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class ABaseLevelController* LevelController;

	UPROPERTY(BlueprintAssignable, Category = EventDispatchers)
	FOnPlayerAction PlayerActionDelegate;

	UPROPERTY(BlueprintAssignable, Category = EventDispatchers)
	FOnPlayerDeath PlayerDeathDelegate;

	UPROPERTY(BlueprintAssignable, Category = EventDispatchers)
	FOnPause PauseDelegate;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Stats | Health")
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Health")
	float MaxHealth;

	// Time of invincibility after taking damage from enemy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category =  "Stats | Health")
	float InvincAfterDamage;

	// Part of enemy HP that adds upon enemy death
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Health")
	float Vampirism;

	// Time between heal bursts while healing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Health")
	float HealBurstTimeBetween;

	// Distance that character covers when dashing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Dash")
	float DashDistance;

	// Time that should be passed between start and end of dash
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Dash")
	float DashTime;

	// Time before dash end when character gets move control back (0.0f for absence of control throughout dash)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Dash")
	float DashMoveControlTime;

	// Time that should pass until character can dash again
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Dash")
	float DashCooldownTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Score")
	float Score;

	// Base number of score earned by killing enemy (that number multiplies by damage multiplier)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Score")
	float BaseScoreForEnemy;

	// Adds to damage multiplier when one enemy starts chasing character
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Attack | Damage Multiplying")
	float DamageMultiplierEnemyCost;

	// Time that should pass until damage multiplier starts to decrease due to decreasing target damage multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Attack | Damage Multiplying")
	float DamageMultiplierStaysTime;

	// Speed of damage multiplier decrease (unitrs per second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Attack | Damage Multiplying")
	float DamageMultiplierDecreaseSpeed;

	// Current damage multiplier
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats | Attack | Damage Multiplying")
	float DamageMultiplier;

	// Damage multiplier that calculated as enemy chasing count multiplied by DamageMultiplierEnemyCost
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats | Attack | Damage Multiplying")
	float TargetDamageMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Attack")
	FPlayerAttackStats SimpleAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bIsDebugOn;

	// Time for which debug damage indicator becames visible
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	float DebugDamageIndicatorTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsGamePaused;

	// Time for which damage FX shown
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	float DamageFXTime;

	// Alpha of damage vignette
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	float DamageFXAlpha;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraFovChangeSpeed;

	// Fov multiplier when speed buffed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float SpeedBuffCameraFovMultiplicator;

	// Percentage of UI dash bar
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	float DashBarPercentage;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	EPlayerMovementPhase MovementPhase;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	EPlayerAttackPhase AttackPhase;

protected:

	class UCharacterMovementComponent* MoveComp;
	class UInputComponent* InputComp;

	bool bCanDash;
	bool bIsDashMovementBlocked;
	FVector DashDestination;
	float DashTimer;
	float DashCooldownTimer;

	bool bIsInvincible;
	FTimerHandle InvincTimerHandle;

	bool bDamageMultiplierStays;
	bool bDamageMultiplierFalling;
	FTimerHandle DamageMultiplierStaysTimerHandle;

	FTimerHandle AttackTimerHandle;

	// Set of enemies that were collided with current attack
	TSet<class ABaseNPCSimpleChase*> AttackEnemiesCollided;

	// Set of enemies that are chasing player
	TSet<class ABaseNPCSimpleChase*> EnemyChasing;

	float DamageFXTimer;

	FTimerHandle DebugDamageIndicatorTimerHandle;

	float BaseSpeed;
	float BaseJumpVelocity;
	float BaseCameraFov;
	float TargetCameraFov;
	FTimerHandle SpeedBuffTimerHandle;

	bool bIsHealing;
	float HealPerBurst; 
	float HealRemaining;
	FTimerHandle HealBuffTimerHandle;

protected:

	void MoveForward(float Value);
	void MoveRight(float Value);

	// f(x) where int_0^1(f(x))dx = 1 that defines dash velocity value during dash time
	static inline float DashVelocityCurve(float x);

	// [0, 1] -> [0, 1] function that defines alpha of damage FX vignette on time
	static inline float DamageFXCurve(float x);

	// On end of invincibility after damage
	void OnEndInvincibility();

	void OnEndDamageMultiplierStays();

	void Dash();
	void AllowMovingWhileDash();
	void StopDashing();
	void OnEndDashCooldown();

	void SetAttackCollision(bool bToActivate);
	void SetDebugAttackCollision(bool bToActivate);

	// Execute simple attack
	void Attack();
	void OnEndAttack();

	void OnEndSpeedBuff();

	void HealBurst();
	void OnEndHealBuff();

	void ActivateDebugDamageIndicator();
	void OnEndDebugDamageIndicatorTimer();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void DashTick(float DeltaSeconds);
	void AttackTick();

public:

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE class ABaseLevelController* GetLevelController() const { return LevelController; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE int GetEnemyChasingCount() const { return EnemyChasing.Num(); }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE TArray<class ABaseNPCSimpleChase*> GetEnemyChasingArray() const { return EnemyChasing.Array(); }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE TSet<class ABaseNPCSimpleChase*> GetEnemyChasingSet() const { return EnemyChasing; }

	UFUNCTION(BlueprintCallable)
	void TakeDamage(float Damage);

	// Called when health is below zero
	UFUNCTION(BlueprintCallable)
	void PlayDeath();

	UFUNCTION(BlueprintCallable)
	void SetMouseCursorShow(bool bToShow);

	UFUNCTION(BlueprintCallable)
	void Pause();

	UFUNCTION(BlueprintCallable)
	void ReceiveAttackInput();

	UFUNCTION(BlueprintCallable)
	void SetSpeedBuff(float SpeedMult, float JumpMult, float Time);

	UFUNCTION(BlueprintCallable)
	void SetHealBuff(float Heal, int BurstCount);

	UFUNCTION(BlueprintCallable)
	void UpdateDamageMultiplier();

	void OnEnemyAggro(class ABaseNPCSimpleChase* Enemy);

	void OnEnemyDeath(class ABaseNPCSimpleChase* Enemy);

	UFUNCTION(BlueprintCallable)
	void RemoveEnemyChasing(class ABaseNPCSimpleChase* Enemy);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetPowerVFXAlpha();
};
