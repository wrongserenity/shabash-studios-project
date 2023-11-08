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

public:

	ABase_NPC_SimpleChase();

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Attack")
	FAttackStats SimpleAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	float Debug_DamageIndicatorTime;

protected:

	uint8 TickSemaphore;

	FTimerHandle AttackTimerHandle;
	EAttackPhase Phase;
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
	void Attack();

	UFUNCTION(BlueprintCallable)
	void JumpTo(FVector Destination);

	UFUNCTION(BlueprintCallable)
	void PlayDeath();
};
