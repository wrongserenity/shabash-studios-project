#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Base_NPC_SimpleChase.generated.h"

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
	NotAttacking UMETA(DisplayName="NotAttacking"),
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

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyDamaged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttackEnd, bool, success);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJumpEnd, bool, success);

UCLASS()
class HYPERCUBE_API ABase_NPC_SimpleChase : public ACharacter
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
	class UBoxComponent* Debug_AttackCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* Debug_DamageIndicator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* SlowDebuffEffectWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* DamageDebuffEffectWidget;

public:

	ABase_NPC_SimpleChase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = LevelController)
	class ABase_LevelController* LevelController;

	UPROPERTY(BlueprintAssignable, Category = EventDispatchers)
	FOnEnemyDamaged EnemyDamagedDelegate;

	UPROPERTY(BlueprintAssignable, Category = EventDispatchers)
	FOnEnemyDeath EnemyDeathDelegate;

	UPROPERTY(BlueprintAssignable, Category = EventDispatchers)
	FOnAttackEnd AttackEndDelegate;

	UPROPERTY(BlueprintAssignable, Category = EventDispatchers)
	FOnJumpEnd JumpEndDelegate;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDebug;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	float Debug_DamageIndicatorTime;

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

	void SetTickState(bool Activate);
	void ForceTickDisable();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void TickRotateToTarget(float DeltaSeconds);
	void TickMoveForward(float DeltaSeconds);
	void CheckPlayerHit();

	FTimerHandle Debug_DamageIndicatorTimerHandle;
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
	inline class ABase_LevelController* GetLevelController() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	class USphereComponent* GetNoticeCollision() const;

	UFUNCTION(BlueprintCallable)
	void SetSlowDebuff(float Mult, float Time);

	UFUNCTION(BlueprintCallable)
	void SetDamageDebuff(float Mult, float Time);
};
