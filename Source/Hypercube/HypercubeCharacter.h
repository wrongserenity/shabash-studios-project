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
	float DashRecoverTime;

protected:

	class UCharacterMovementComponent* MoveComp;
	class UInputComponent* InputComp;

	EPlayerMovementPhase Phase;

	uint8 TickSemaphore;

	bool bCanDash;
	FVector DashDestination;
	float DashTimer;

	FTimerHandle DashRecoveryTimerHandle;

	bool bIsInvincible;
	FTimerHandle InvincTimerHandle;

protected:

	void MoveForward(float Value);
	void MoveRight(float Value);

	void Dash();
	void StopDashing();
	void OnEndDashRecovery();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaSeconds) override;

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
};

