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
	FOnAttackEnd AttackEnd;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Attack")
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Attack")
	float OpenerTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Attack")
	float AttackTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats | Attack")
	float AfterAttackTime;


protected:

	virtual void BeginPlay() override;

public:	

	UFUNCTION(BlueprintCallable)
	void SetAttackCollision(bool active);
};
