// Copyright Epic Games, Inc. All Rights Reserved.

#include "HypercubeCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/UnrealMathUtility.h"
#include "Components/BoxComponent.h"
#include "BaseNPCSimpleChase.h"
#include "Components/StaticMeshComponent.h"
#include "BaseLevelController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"

// Base class for player Character

AHypercubeCharacter::AHypercubeCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	Capsule = GetCapsuleComponent();
	Capsule->InitCapsuleSize(42.0f, 96.0f);

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
	AttackPhase = EPlayerAttackPhase::None;
	
	bCanDash = true;
	bIsDashMovementBlocked = true;

	Health = MaxHealth = 100.0f;
	InvincAfterDamage = 1.0f;
	Vampirism = 0.0f;
	bIsInvincible = false;

	HealBurstTimeBetween = 1.0f;
	HealRemaining = 0.0f;
	bIsHealing = false;

	DashDistance = 700.0f;
	DashTime = DashTimer = 0.2f;
	DashMoveControlTime = 0.1f;
	DashCooldownTime = DashCooldownTimer = 1.5f;

	Score = 0.0f;
	BaseScoreForEnemy = 10.0f;

	DamageMultiplierEnemyCost = 0.5f;
	EnemyChasing.Empty();
	DamageMultiplier = TargetDamageMultiplier = 1.0f;
	DamageMultiplierStaysTime = 5.0f;
	DamageMultiplierDecreaseSpeed = 1.0f;
	bDamageMultiplierStays = false;
	bDamageMultiplierFalling = false;

	SimpleAttack = { 25.0f, 0.1f, 0.2f, 0.1f, 90.0f, 68.0f };

	AttackCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Attack Collision"));
	AttackCollision->SetupAttachment(RootComponent);
	AttackCollision->SetRelativeLocation(FVector(SimpleAttack.AttackRadius / 2.0f + Capsule->GetUnscaledCapsuleRadius() / 2.0f, 0.0f, 20.0f));
	AttackCollision->SetBoxExtent(FVector(SimpleAttack.AttackRadius - Capsule->GetUnscaledCapsuleRadius(), SimpleAttack.AttackRadius * FMath::Tan(SimpleAttack.AttackAngle * PI / 360.0f), 32.0f));
	AttackCollision->SetGenerateOverlapEvents(false);
	AttackCollision->SetHiddenInGame(false);
	AttackCollision->SetVisibility(false);

	DebugAttackCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Debug Attack Collision"));
	DebugAttackCollision->SetupAttachment(RootComponent);
	DebugAttackCollision->SetRelativeLocation(FVector(SimpleAttack.AttackRadius / 2.0f + GetCapsuleComponent()->GetUnscaledCapsuleRadius() / 2.0f, 0.0f, 20.0f));
	DebugAttackCollision->SetBoxExtent(FVector(SimpleAttack.AttackRadius - GetCapsuleComponent()->GetUnscaledCapsuleRadius(), SimpleAttack.AttackRadius * FMath::Tan(SimpleAttack.AttackAngle * PI / 360.0f), 32.0f));
	DebugAttackCollision->SetGenerateOverlapEvents(true);
	DebugAttackCollision->SetHiddenInGame(false);
	DebugAttackCollision->SetVisibility(false);

	DebugDamageIndicator = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Debug Damage Indicator"));
	DebugDamageIndicator->SetupAttachment(RootComponent);
	DebugDamageIndicator->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
	DebugDamageIndicator->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DebugDamageIndicator->SetGenerateOverlapEvents(false);
	DebugDamageIndicator->SetVisibility(false);

	DebugDamageIndicatorTime = 3.0f;

	//DelayedInitTime = 0.2f;

	bIsGamePaused = false;

	DamageFXAlpha = 0.0f;
	DamageFXTime = 0.5f;
	DamageFXTimer = 0.0f;

	bIsDebugOn = false;

	TargetCameraFov = FollowCamera->FieldOfView;
	CameraFovChangeSpeed = 10.0f;
	SpeedBuffCameraFovMultiplicator = 1.3f;

	HealBuffParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Health Buff Particle System"));
	HealBuffParticleSystem->SetupAttachment(RootComponent);
	HealBuffParticleSystem->SetRelativeLocation(FVector(0.0f, 0.0f, -Capsule->GetScaledCapsuleHalfHeight()));
	HealBuffParticleSystem->SetAutoActivate(false);
	HealBuffParticleSystem->SetActive(false);

	SpeedBuffNiagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Speed Buff Niagara"));
	SpeedBuffNiagara->SetupAttachment(GetMesh());
	SpeedBuffNiagara->SetAutoActivate(false);
	SpeedBuffNiagara->SetActive(false);

	DashBarPercentage = 1.0f;
}

void AHypercubeCharacter::BeginPlay()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseLevelController::StaticClass(), FoundActors);
	if (FoundActors.Num())
	{
		LevelController = Cast<ABaseLevelController>(FoundActors[0]);
		LevelController->SetPlayerCharacter(this);
	}
	else
	{
		LevelController = nullptr;
	}
	PlayerController = GetWorld()->GetFirstPlayerController();

	Super::BeginPlay();
}

void AHypercubeCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	InputComp = PlayerInputComponent;
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &AHypercubeCharacter::Dash);

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AHypercubeCharacter::ReceiveAttackInput);

	PlayerInputComponent->BindAction("Pause", IE_Pressed, this, &AHypercubeCharacter::Pause).bExecuteWhenPaused = true;

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
		DashTick(DeltaSeconds);
	}
	else if (DashCooldownTimer < DashCooldownTime)
	{
		DashCooldownTimer += DeltaSeconds;
		DashBarPercentage = DashCooldownTimer / DashCooldownTime - 0.1f;
		if (DashCooldownTimer >= DashCooldownTime)
		{
			OnEndDashCooldown();
		}
	}
	if (AttackPhase == EPlayerAttackPhase::Attacking)
	{
		AttackTick();
	}
	if (bDamageMultiplierFalling && DamageMultiplier > TargetDamageMultiplier)
	{
		DamageMultiplier -= DamageMultiplierDecreaseSpeed * DeltaSeconds;
		if (DamageMultiplier < TargetDamageMultiplier)
		{
			DamageMultiplier = TargetDamageMultiplier;
			bDamageMultiplierFalling = false;
		}
	}
	if (DamageFXTimer > 0.0f)
	{
		DamageFXAlpha = DamageFXCurve(1.0f - DamageFXTimer / DamageFXTime);
		DamageFXTimer -= DeltaSeconds;
		if (DamageFXTimer <= 0.0f)
		{
			DamageFXAlpha = 0.0f;
		}
	}
	if (FollowCamera->FieldOfView != TargetCameraFov)
	{
		FollowCamera->FieldOfView = FMath::Lerp(FollowCamera->FieldOfView, TargetCameraFov, CameraFovChangeSpeed * DeltaSeconds);
		if (FMath::IsNearlyEqual(FollowCamera->FieldOfView, TargetCameraFov, 0.001f))
		{
			FollowCamera->FieldOfView = TargetCameraFov;
		}
	}
	if (bIsHealing)
	{
		HealBuffParticleSystem->SetWorldRotation(FRotator::ZeroRotator, false);
	}

	Super::Tick(DeltaSeconds);
}

void AHypercubeCharacter::DashTick(float DeltaSeconds)
{
	AddActorWorldOffset(DashDestination * (DashDistance / DashTime) * DashVelocityCurve(DashTimer / DashTime) * DeltaSeconds, true);
	DashTimer += DeltaSeconds;
	DashBarPercentage = 1.0f - DashTimer / DashTime;
	if (bIsDashMovementBlocked && DashTime - DashTimer <= DashMoveControlTime)
	{
		AllowMovingWhileDash();
	}
	if (DashTimer >= DashTime)
	{
		StopDashing();
	}
}

void AHypercubeCharacter::AttackTick()
{
	TSet<AActor*> Collisions;
	AttackCollision->GetOverlappingActors(Collisions, ABaseNPCSimpleChase::StaticClass());
	for (AActor* CollidedActor : Collisions)
	{
		ABaseNPCSimpleChase* ActorNPC = Cast<ABaseNPCSimpleChase>(CollidedActor);
		if (ActorNPC && !AttackEnemiesCollided.Contains(ActorNPC))
		{
			UE_LOG(LogTemp, Warning, TEXT("Enemy damaged!"));
			ActorNPC->TakeDamage(SimpleAttack.Damage * DamageMultiplier);
			AttackEnemiesCollided.Add(ActorNPC);
		}
	}
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

inline float AHypercubeCharacter::DashVelocityCurve(float x)
{
	return (x >= 0.0f && x <= 1.0f) ? (PI / 2.0f) * FMath::Sin(PI * x) : 0.0f;
}

inline float AHypercubeCharacter::DamageFXCurve(float x)
{
	if (x >= 0.0f && x <= 1.0f)
	{
		if (x < 0.2f)
		{
			return 2.5f * x;
		}
		if (x < 0.5f)
		{
			return 0.5f;
		}
		return 1.0f - x;
	}
	return 0.0f;
}

void AHypercubeCharacter::Dash()
{
	if (!bCanDash || MovementPhase != EPlayerMovementPhase::Walking)
	{
		return;
	}
	PlayerActionDelegate.Broadcast(EPlayerAction::Dash);
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
	
	Capsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
}

void AHypercubeCharacter::AllowMovingWhileDash()
{
	bIsDashMovementBlocked = false;
	MoveComp->SetMovementMode(EMovementMode::MOVE_Flying);
}

void AHypercubeCharacter::StopDashing()
{
	MoveComp->SetMovementMode(EMovementMode::MOVE_Walking);
	MovementPhase = EPlayerMovementPhase::Walking;
	bIsDashMovementBlocked = true;
	Capsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	DashCooldownTimer = 0.0f;
}

void AHypercubeCharacter::OnEndDashCooldown()
{
	bCanDash = true;
}

void AHypercubeCharacter::SetAttackCollision(bool bToActivate)
{
	if (bIsDebugOn)
	{
		AttackCollision->SetVisibility(bToActivate);
	}
	AttackCollision->SetActive(bToActivate);
}

void AHypercubeCharacter::SetDebugAttackCollision(bool bToActivate)
{
	if (bIsDebugOn)
	{
		DebugAttackCollision->SetVisibility(bToActivate);
		DebugAttackCollision->SetActive(bToActivate);
	}
}

void AHypercubeCharacter::ReceiveAttackInput()
{
	if (MovementPhase != EPlayerMovementPhase::Walking)
	{
		return;
	}
	MovementPhase = EPlayerMovementPhase::Attacking;
	PlayerActionDelegate.Broadcast(EPlayerAction::Attack);
	Attack();
}

void AHypercubeCharacter::Attack()
{
	switch (AttackPhase)
	{
	case EPlayerAttackPhase::None:
		AttackPhase = EPlayerAttackPhase::Opener;
		SetDebugAttackCollision(true);
		GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &AHypercubeCharacter::Attack, SimpleAttack.OpenerTime, false);
		break;
	case EPlayerAttackPhase::Opener:
		AttackPhase = EPlayerAttackPhase::Attacking;
		AttackEnemiesCollided.Empty();
		SetDebugAttackCollision(false);
		SetAttackCollision(true);
		GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &AHypercubeCharacter::Attack, SimpleAttack.AttackTime, false);
		break;
	case EPlayerAttackPhase::Attacking:
		AttackPhase = EPlayerAttackPhase::AfterAttack;
		SetAttackCollision(false);
		SetDebugAttackCollision(true);
		GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &AHypercubeCharacter::Attack, SimpleAttack.AfterAttackTime, false);
		break;
	case EPlayerAttackPhase::AfterAttack:
		AttackPhase = EPlayerAttackPhase::None;
		SetDebugAttackCollision(false);
		OnEndAttack();
		break;
	default:
		break;
	}
}

void AHypercubeCharacter::OnEndAttack()
{
	if (MovementPhase != EPlayerMovementPhase::None)
	{
		MovementPhase = EPlayerMovementPhase::Walking;
	}
}

void AHypercubeCharacter::ActivateDebugDamageIndicator()
{
	DebugDamageIndicator->SetVisibility(true);
	if (GetWorld()->GetTimerManager().IsTimerActive(DebugDamageIndicatorTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(DebugDamageIndicatorTimerHandle);
	}
	GetWorld()->GetTimerManager().SetTimer(DebugDamageIndicatorTimerHandle, this, &AHypercubeCharacter::OnEndDebugDamageIndicatorTimer, DebugDamageIndicatorTime, false);
}

void AHypercubeCharacter::OnEndDebugDamageIndicatorTimer()
{
	DebugDamageIndicator->SetVisibility(false);
}

void AHypercubeCharacter::TakeDamage(float Damage)
{
	if (bIsInvincible)
	{
		return;
	}
	Health -= Damage;
	if (bIsDebugOn)
	{
		ActivateDebugDamageIndicator();
	}
	bIsInvincible = true;
	DamageFXTimer = DamageFXTime;
	PlayerActionDelegate.Broadcast(EPlayerAction::Damaged);
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
	MoveComp->SetMovementMode(EMovementMode::MOVE_None);
	bCanDash = false;
	LevelController->OnPlayerDeath();
	PlayerDeathDelegate.Broadcast();
}

void AHypercubeCharacter::UpdateDamageMultiplier()
{
	TargetDamageMultiplier = 1.0f;

	for (ABaseNPCSimpleChase* Enemy : EnemyChasing)
	{
		TargetDamageMultiplier += DamageMultiplierEnemyCost * Enemy->GetDamageMultiplierMultiplier();
	}

	if (TargetDamageMultiplier >= DamageMultiplier)
	{
		if (GetWorld()->GetTimerManager().IsTimerActive(DamageMultiplierStaysTimerHandle))
		{
			GetWorld()->GetTimerManager().ClearTimer(DamageMultiplierStaysTimerHandle);
		}
		DamageMultiplier = TargetDamageMultiplier;
	}
	else
	{
		if (!GetWorld()->GetTimerManager().IsTimerActive(DamageMultiplierStaysTimerHandle) && !bDamageMultiplierFalling)
		{
			GetWorld()->GetTimerManager().SetTimer(DamageMultiplierStaysTimerHandle, this, &AHypercubeCharacter::OnEndDamageMultiplierStays, DamageMultiplierStaysTime, false);
			bDamageMultiplierStays = true;
		}
	}
}

void AHypercubeCharacter::OnEndDamageMultiplierStays()
{
	bDamageMultiplierStays = false;
	bDamageMultiplierFalling = true;
}

void AHypercubeCharacter::OnEnemyAggro(class ABaseNPCSimpleChase* Enemy)
{
	if (!EnemyChasing.Contains(Enemy))
	{
		EnemyChasing.Add(Enemy);
		UpdateDamageMultiplier();
	}

	LevelController->UpdateStackState();
}

void AHypercubeCharacter::OnEnemyDeath(class ABaseNPCSimpleChase* Enemy)
{
	Score += BaseScoreForEnemy * DamageMultiplier;
	Health += Vampirism * Enemy->MaxHealth;
	Health = Health > MaxHealth ? MaxHealth : Health;
	if (LevelController)
	{
		LevelController->RemoveEnemy(Enemy);
		LevelController->UpdateMaxMultiplicator(DamageMultiplier);
	}
	if (EnemyChasing.Contains(Enemy))
	{
		EnemyChasing.Remove(Enemy);
		UpdateDamageMultiplier();
	}

	LevelController->UpdateStackState();
}

void AHypercubeCharacter::RemoveEnemyChasing(ABaseNPCSimpleChase* Enemy)
{
	EnemyChasing.Remove(Enemy);
	UpdateDamageMultiplier();

	LevelController->UpdateStackState();
}

void AHypercubeCharacter::SetMouseCursorShow(bool bToShow)
{
	PlayerController->bShowMouseCursor = bToShow;
	PlayerController->bEnableClickEvents = bToShow;
	PlayerController->bEnableMouseOverEvents = bToShow;
	if (bToShow)
	{
		PlayerController->SetInputMode(FInputModeGameAndUI());
	}
	else
	{
		PlayerController->SetInputMode(FInputModeGameOnly());
	}
}

void AHypercubeCharacter::Pause()
{
	bIsGamePaused = !bIsGamePaused;
	UGameplayStatics::SetGamePaused(GetWorld(), bIsGamePaused);
	SetMouseCursorShow(bIsGamePaused);
	PauseDelegate.Broadcast(bIsGamePaused);
}

void AHypercubeCharacter::SetSpeedBuff(float SpeedMult, float JumpMult, float Time)
{
	if (GetWorld()->GetTimerManager().IsTimerActive(SpeedBuffTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(SpeedBuffTimerHandle);
	}
	else
	{
		BaseSpeed = MoveComp->MaxWalkSpeed;
		BaseJumpVelocity = MoveComp->JumpZVelocity;
		BaseCameraFov = TargetCameraFov;

		MoveComp->MaxWalkSpeed *= SpeedMult;
		MoveComp->JumpZVelocity *= JumpMult;
		TargetCameraFov *= SpeedBuffCameraFovMultiplicator;

		SpeedBuffNiagara->SetActive(true);
	}
	GetWorld()->GetTimerManager().SetTimer(SpeedBuffTimerHandle, this, &AHypercubeCharacter::OnEndSpeedBuff, Time, false);
}

void AHypercubeCharacter::OnEndSpeedBuff()
{
	MoveComp->MaxWalkSpeed = BaseSpeed;
	MoveComp->JumpZVelocity = BaseJumpVelocity;
	TargetCameraFov = BaseCameraFov;
	SpeedBuffNiagara->SetActive(false);
}

void AHypercubeCharacter::SetHealBuff(float Heal, int BurstCount)
{
	HealRemaining = Heal;
	HealPerBurst = Heal / (float)BurstCount;

	if (bIsHealing)
	{
		return;
	}

	bIsHealing = true;
	GetWorld()->GetTimerManager().SetTimer(HealBuffTimerHandle, this, &AHypercubeCharacter::HealBurst, HealBurstTimeBetween, false);
	HealBuffParticleSystem->SetActive(true);
	PlayerActionDelegate.Broadcast(EPlayerAction::HealBuff);
}

void AHypercubeCharacter::HealBurst()
{
	if (HealRemaining <= HealPerBurst)
	{
		HealRemaining = 0.0f;
		Health += HealRemaining;
		if (Health > MaxHealth)
		{
			Health = MaxHealth;
		}

		OnEndHealBuff();
	}
	else
	{
		HealRemaining -= HealPerBurst;
		Health += HealPerBurst;

		if (Health > MaxHealth)
		{
			Health = MaxHealth;
		}

		GetWorld()->GetTimerManager().SetTimer(HealBuffTimerHandle, this, &AHypercubeCharacter::HealBurst, HealBurstTimeBetween, false);
	}

	PlayerActionDelegate.Broadcast(EPlayerAction::HealBurst);
}

void AHypercubeCharacter::OnEndHealBuff()
{
	UE_LOG(LogTemp, Warning, TEXT("Player Healing Ends!"));
	bIsHealing = false;
	HealBuffParticleSystem->SetActive(false);
	PlayerActionDelegate.Broadcast(EPlayerAction::HealBuffEnd);
}

float AHypercubeCharacter::GetPowerVFXAlpha()
{
	if (DamageMultiplier <= 3.0f)
	{
		return 0.0f;
	}
	if (DamageMultiplier <= 5.0f)
	{
		return (DamageMultiplier - 3.0f) / 4.0f;
	}
	if (DamageMultiplier <= 50.0f)
	{
		return 0.5f + (DamageMultiplier - 5.0f) / 90.0f;
	}
	return 1.0f;
}