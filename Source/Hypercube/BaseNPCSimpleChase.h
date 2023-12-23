#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseNPCSimpleChase.generated.h"

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackRotationMultiplier;

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
	DamageDecreaseDebuffEnd UMETA(DisplayName = "AttackDecreaseDebuffEnd")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyAction, EEnemyAction, Action, bool, Success);

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* DebugDamageIndicator;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	//class UWidgetComponent* SlowDebuffEffectWidget;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	//class UWidgetComponent* DamageDebuffEffectWidget;

public:

	ABaseNPCSimpleChase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = LevelController)
	class ABaseLevelController* LevelController;

	UPROPERTY(BlueprintAssignable, Category = EventDispatchers)
	FOnEnemyDeath EnemyDeathDelegate;

	UPROPERTY(BlueprintAssignable, Category = EventDispatchers)
	FOnEnemyAction EnemyActionDelegate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Health")
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Health")
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Jump")
	float JumpTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Aggro")
	float AggroRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Aggro")
	float AggroTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Attack")
	FAttackStats SimpleAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Unstuck")
	float UnstuckPlayerSightUpdate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Unstuck")
	float UnstuckAroundPlayerRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Unstuck")
	int MaxAttempsToUnstuck;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bIsDebugOn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	float DebugDamageIndicatorTime;

protected:

	uint8 TickSemaphore;

	FTimerHandle NoticeTimerHandle;
	EEnemyPhase MovePhase;

	FTimerHandle AttackTimerHandle;
	EAttackPhase AttackPhase;
	class AHypercubeCharacter* AttackTarget;

	FTimerHandle DelayedInitTimerHandle;
	float DelayedInitTime;
	void DelayedInit();

	void SetTickState(bool bToActivate);
	void ForceTickDisable();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void TickRotateToTarget(float DeltaSeconds);
	void TickMoveForward(float DeltaSeconds);
	void CheckPlayerHit();

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

	FTimerHandle CheckPlayerSightTimerHandle;

public:

	UFUNCTION(BlueprintCallable)
	void SetAttackCollision(bool Active);

	UFUNCTION(BlueprintCallable)
	void SetDebugAttackCollision(bool Active);

	UFUNCTION(BlueprintCallable)
	void TakeDamage(float Damage);

	UFUNCTION(BlueprintCallable)
	void OnNotice();

	UFUNCTION(BlueprintCallable)
	void AfterNotice();

	UFUNCTION(BlueprintCallable)
	void Attack();

	UFUNCTION(BlueprintCallable)
	void JumpTo(FVector Destination);

	UFUNCTION(BlueprintCallable)
	void PlayDeath();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	class ABaseLevelController* GetLevelController() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	class USphereComponent* GetNoticeCollision() const;

	UFUNCTION(BlueprintCallable)
	void SetSlowDebuff(float Mult, float Time);

	UFUNCTION(BlueprintCallable)
	void SetDamageDebuff(float Mult, float Time);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool PlayerHasSightOn() const;

	UFUNCTION(BlueprintCallable)
	void Unstuck();
};
