// Copyright Epic Games, Inc. All Rights Reserved.

#include "HypercubeCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/UnrealMathUtility.h"

//////////////////////////////////////////////////////////////////////////
// AHypercubeCharacter

AHypercubeCharacter::AHypercubeCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	TickSemaphore = 0;
	SetActorTickEnabled(false);

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	MoveComp = GetCharacterMovement();
	MoveComp->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	MoveComp->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	MoveComp->JumpZVelocity = 600.0f;
	MoveComp->AirControl = 1.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	MovementPhase = EPlayerMovementPhase::Walking;

	bCanDash = true;
	bDashMovementBlocked = true;

	Health = MaxHealth = 100.0f;
	InvincAfterDamage = 1.0f;
	bIsInvincible = false;

	DashDistance = 700.0f;
	DashTime = 0.2f;
	DashMoveControlTime = 0.1f;
	DashCooldownTime = 0.5f;
}

//////////////////////////////////////////////////////////////////////////
// Input

void AHypercubeCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	InputComp = PlayerInputComponent;
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &AHypercubeCharacter::Dash);

	PlayerInputComponent->BindAxis("MoveForward", this, &AHypercubeCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AHypercubeCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
}

void AHypercubeCharacter::Tick(float DeltaSeconds)
{
	if (MovementPhase == EPlayerMovementPhase::Dashing)
	{
		AddActorWorldOffset(DashDestination * (DashDistance / DashTime) * DashVelocityCurve(DashTimer / DashTime) * DeltaSeconds, true);
		DashTimer += DeltaSeconds;
		if (bDashMovementBlocked && DashTime - DashTimer <= DashMoveControlTime)
		{
			AllowMovingWhileDash();
		}
		if (DashTimer >= DashTime)
		{
			StopDashing();
		}
	}
	Super::Tick(DeltaSeconds);
}

void AHypercubeCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AHypercubeCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

float AHypercubeCharacter::DashVelocityCurve(float x) // f(x) where int_0^1(f(x))dx = 1
{
	return (x >= 0.0f && x <= 1.0f) ? (PI / 2.0f) * FMath::Sin(PI * x) : 0.0f;
	//return (x < 0.5f ? 4.0f * x : 4.0f - 4.0f * x);
	//return 1.0;
}

void AHypercubeCharacter::Dash()
{
	if (!bCanDash || MovementPhase == EPlayerMovementPhase::None)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Can not dash!"));
		return;
	}
	FVector Forward = FollowCamera->GetForwardVector();
	Forward.Z = 0.0f;
	Forward.Normalize();
	FVector Right = FollowCamera->GetRightVector();
	Right.Z = 0.0f;
	Right.Normalize();
	DashDestination = Forward * InputComp->GetAxisValue(TEXT("MoveForward")) + Right * InputComp->GetAxisValue(TEXT("MoveRight"));
	if (DashDestination.IsNearlyZero())
	{
		DashDestination = GetActorForwardVector();
	}
	else
	{
		DashDestination.Normalize();
	}
	SetActorRotation(UKismetMathLibrary::MakeRotFromXZ(DashDestination, FVector::ZAxisVector));
	DashTimer = 0.0f;
	MoveComp->SetMovementMode(EMovementMode::MOVE_None);
	MovementPhase = EPlayerMovementPhase::Dashing;
	bCanDash = false;

	//TSet<AActor*> collisions;
	//GetCapsuleComponent()->GetOverlappingActors(collisions);
	//UE_LOG(LogTemp, Warning, TEXT("%d"), collisions.Num());
	//for (auto it = collisions.begin(); it != collisions.end(); ++it)
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("%s"), *(*it)->GetName());
	//}
}

void AHypercubeCharacter::AllowMovingWhileDash()
{
	bDashMovementBlocked = false;
	MoveComp->SetMovementMode(EMovementMode::MOVE_Flying);
}

void AHypercubeCharacter::StopDashing()
{
	MoveComp->SetMovementMode(EMovementMode::MOVE_Walking);
	MovementPhase = EPlayerMovementPhase::Walking;
	bDashMovementBlocked = true;
	//UE_LOG(LogTemp, Warning, TEXT("End of dash"));
	GetWorld()->GetTimerManager().SetTimer(DashCooldownTimerHandle, this, &AHypercubeCharacter::OnEndDashCooldown, DashCooldownTime, false);
}

void AHypercubeCharacter::OnEndDashCooldown()
{
	//UE_LOG(LogTemp, Warning, TEXT("End of dash cooldown"));
	bCanDash = true;
}

void AHypercubeCharacter::TakeDamage(float Damage)
{
	if (bIsInvincible)
	{
		return;
	}
	Health -= Damage;
	bIsInvincible = true;
	UE_LOG(LogTemp, Warning, TEXT("Damage: %f, Now Health: %f"), Damage, Health);
	if (Health <= 0.0f)
	{
		PlayDeath();
		return;
	}
	GetWorld()->GetTimerManager().SetTimer(InvincTimerHandle, this, &AHypercubeCharacter::OnEndInvincibility, InvincAfterDamage, false);
}

void AHypercubeCharacter::OnEndInvincibility()
{
	bIsInvincible = false;
}

void AHypercubeCharacter::PlayDeath()
{
	MovementPhase = EPlayerMovementPhase::None;
	bCanDash = false;
}
