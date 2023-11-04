// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HypercubeCharacter.generated.h"

UENUM(BlueprintType)
enum class EPlayerPhase : uint8
{
	None UMETA(DisplayName = "None"),
	Walking UMETA(DisplayName = "Walking"),
	Dashing UMETA(DisplayName = "Dashing"),
	AttackOpener UMETA(DisplayName = "AttackOpener"),
	Attacking UMETA(DisplayName = "Attacking"),
	AfterAttack UMETA(DisplayName = "AfterAttack")
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
	class UBoxComponent* AttackCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* Debug_AttackCollision;

public:
	AHypercubeCharacter();

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Dash")
	float DashDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Dash")
	float DashTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Dash")
	float DashMoveControlTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Dash")
	float DashCooldownTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Attack | Damage Multiplying")
	float DamageMultiplierEnemyCost;

	UPROPERTY(BlueprintReadOnly)
	float DamageMulptiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Attack")
	FPlayerAttackStats SimpleAttack;

protected:

	class UCharacterMovementComponent* MoveComp;
	class UInputComponent* InputComp;

	EPlayerPhase Phase;

	uint8 TickSemaphore;

	bool bCanDash;
	bool bDashMovementBlocked;
	FVector DashDestination;
	float DashTimer;

	FTimerHandle DashCooldownTimerHandle;

	bool bIsInvincible;
	FTimerHandle InvincTimerHandle;

	FTimerHandle AttackTimerHandle;

	TSet<class ABase_NPC_SimpleChase*> AttackEnemiesCollided;

	TSet<class ABase_NPC_SimpleChase*> EnemyChasing;

protected:

	void MoveForward(float Value);
	void MoveRight(float Value);

	inline float DashVelocityCurve(float x); // f(x) where int_0^1(f(x))dx = 1

	void Dash();
	void AllowMovingWhileDash();
	void StopDashing();
	void OnEndDashCooldown();

	void SetAttackCollision(bool Activate);
	void SetDebugAttackCollision(bool Activate);
	void Attack();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
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
	void AddChasingDamageMultiplier(class ABase_NPC_SimpleChase* Enemy);

	UFUNCTION(BlueprintCallable)
	void RemoveChasingDamageMultiplier(class ABase_NPC_SimpleChase* Enemy);
};

