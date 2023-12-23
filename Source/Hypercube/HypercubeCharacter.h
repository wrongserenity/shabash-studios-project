// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HypercubeCharacter.generated.h"

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
	Damaged UMETA(DisplayName = "Damaged")
};

USTRUCT(BlueprintType)
struct FPlayerAttackStats
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackMoveForwardSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackRadius;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCapsuleComponent* Capsule;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* AttackCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* Debug_AttackCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* Debug_DamageIndicator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class APlayerController* PlayerController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* SpeedBuffEffectWidget;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category =  "Stats | Health")
	float InvincAfterDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Health")
	float Vampirism;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Dash")
	float DashDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Dash")
	float DashTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Dash")
	float DashMoveControlTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Dash")
	float DashCooldownTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Score")
	float Score;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Score")
	float BaseScoreForEnemy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Attack | Damage Multiplying")
	float DamageMultiplierEnemyCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Attack | Damage Multiplying")
	float DamageMultiplierStaysTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Attack | Damage Multiplying")
	float DamageMultiplierDecreaseSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats | Attack | Damage Multiplying")
	float DamageMultiplier;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats | Attack | Damage Multiplying")
	float TargetDamageMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Attack")
	FPlayerAttackStats SimpleAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDebug;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	float Debug_DamageIndicatorTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsGamePaused;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	float DamageFXTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	float DamageFXAlpha;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraFovChangeSpeed;

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
	bool bDashMovementBlocked;
	FVector DashDestination;
	float DashTimer;
	float DashCooldownTimer;

	bool bIsInvincible;
	FTimerHandle InvincTimerHandle;

	bool bDamageMultiplierStays;
	bool bDamageMultiplierFalling;
	FTimerHandle DamageMultiplierStaysTimerHandle;

	FTimerHandle AttackTimerHandle;

	TSet<class ABaseNPCSimpleChase*> AttackEnemiesCollided;

	TSet<class ABaseNPCSimpleChase*> EnemyChasing;

	float DamageFXTimer;

	FTimerHandle Debug_DamageIndicatorTimerHandle;

	float BaseSpeed;
	float BaseJumpVelocity;
	float BaseCameraFov;
	float TargetCameraFov;
	FTimerHandle SpeedBuffTimerHandle;

protected:

	//FTimerHandle DelayedInitTimerHandle;
	//float DelayedInitTime;
	//void DelayedInit();

	void MoveForward(float Value);
	void MoveRight(float Value);

	inline float DashVelocityCurve(float x); // f(x) where int_0^1(f(x))dx = 1
	inline float DamageFXCurve(float x);

	void Dash();
	void AllowMovingWhileDash();
	void StopDashing();
	void OnEndDashCooldown();

	void SetAttackCollision(bool Activate);
	void SetDebugAttackCollision(bool Activate);
	void Attack();
	void OnEndAttack();

	void ActivateDebugDamageIndicator();
	void OnEndDebugDamageIndicatorTimer();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void DashTick(float DeltaSeconds);
	void AttackTick();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(BlueprintCallable)
	void TakeDamage(float Damage);

	void OnEndInvincibility();

	UFUNCTION(BlueprintCallable)
	void PlayDeath();

	UFUNCTION(BlueprintCallable)
	void UpdateDamageMultiplier();

	UFUNCTION(BlueprintCallable)
	void OnEndDamageMultiplierStays();

	UFUNCTION(BlueprintCallable)
	void OnEnemyAggro(class ABaseNPCSimpleChase* Enemy);

	UFUNCTION(BlueprintCallable)
	void OnEnemyDeath(class ABaseNPCSimpleChase* Enemy);

	UFUNCTION(BlueprintCallable)
	void SetMouseCursorShow(bool Activate);

	UFUNCTION(BlueprintCallable)
	void Pause();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	class ABaseLevelController* GetLevelController() const;

	UFUNCTION(BlueprintCallable)
	void ReceiveAttackInput();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	int GetEnemyChasingCount() const;

	UFUNCTION(BlueprintCallable)
	void SetSpeedBuff(float SpeedMult, float JumpMult, float Time);

	void OnEndSpeedBuff();
};

