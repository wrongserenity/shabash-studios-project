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

UCLASS()
class HYPERCUBE_API ABase_NPC_SimpleChase : public ACharacter
{
	GENERATED_BODY()

public:

	ABase_NPC_SimpleChase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	class UCapsuleComponent* Capsule;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	class UCharacterMovementComponent* MoveComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	class UBoxComponent* AttackCollision;

	UPROPERTY(BlueprintAssignable, Category = EventDispatchers)
	FOnAttackEnd AttackEndDelegate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Attack")
	FAttackStats SimpleAttack;

protected:

	FTimerHandle AttackTimerHandle;
	EAttackPhase Phase;
	class AHypercubeCharacter* AttackTarget;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void TickRotateToTarget(float DeltaSeconds);
	void TickMoveForward(float DeltaSeconds);
	void CheckPlayerHit();

public:	

	UFUNCTION(BlueprintCallable)
	void SetAttackCollision(bool Active);

	UFUNCTION(BlueprintCallable)
	void Attack();
};
