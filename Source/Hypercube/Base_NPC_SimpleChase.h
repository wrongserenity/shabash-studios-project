#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Base_NPC_SimpleChase.generated.h"

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
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Attack")
	float OpenerTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Attack")
	float AttackTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Attack")
	float AfterAttackTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Attack")
	float AttackRotationMultiplier;

protected:

	FTimerHandle AttackTimerHandle;

	enum class AttackPhase { NotAttacking, Opener, Attacking, AfterAttack };
	AttackPhase Phase;

	class AHypercubeCharacter* AttackTarget;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	void TickRotateToTarget(float DeltaSeconds);

public:	

	UFUNCTION(BlueprintCallable)
	void SetAttackCollision(bool Active);

	UFUNCTION(BlueprintCallable)
	void Attack();
};
