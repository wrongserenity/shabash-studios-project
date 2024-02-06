#include "BaseNPCSimpleChase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Math/UnrealMathVectorCommon.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "HypercubeCharacter.h"
#include "Components/SphereComponent.h"
#include "BaseLevelController.h"
#include "Components/WidgetComponent.h"
#include "NavMesh/RecastNavMesh.h"
#include "NavigationSystem.h"

// Base class for enemy NPC

ABaseNPCSimpleChase::ABaseNPCSimpleChase()
{
	PrimaryActorTick.bCanEverTick = true;
	DelayedInitTime = 0.1f;

	bUseControllerRotationYaw = false;

	Capsule = GetCapsuleComponent();
	Capsule->InitCapsuleSize(42.0f, 96.0f);

	MoveComp = GetCharacterMovement();
	MoveComp->JumpZVelocity = 560.0f;
	MoveComp->bOrientRotationToMovement = true;

	Health = MaxHealth = 100.0f;

	JumpTime = 2.0f;

	AggroRadius = 800.0f;
	AggroTime = 0.5f;

	NoticeCollision = CreateAbstractDefaultSubobject<USphereComponent>(TEXT("Notice Collision"));
	NoticeCollision->AttachTo(RootComponent);
	NoticeCollision->SetSphereRadius(AggroRadius);
	NoticeCollision->SetGenerateOverlapEvents(false);

	SimpleAttack = { 25.0f, 0.4f, 0.3f, 0.2f, 7.5f, 150.0f, 75.0f, 35.0f };

	AttackCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackCollision"));
	AttackCollision->AttachTo(RootComponent);
	AttackCollision->SetRelativeLocation(FVector((Capsule->GetScaledCapsuleRadius() + SimpleAttack.AttackLength) / 2.0f, 0.0f, 20.0f));
	AttackCollision->SetBoxExtent(FVector(SimpleAttack.AttackLength - Capsule->GetScaledCapsuleRadius(), SimpleAttack.AttackWidth, 32.0f));
	AttackCollision->SetGenerateOverlapEvents(true);
	AttackCollision->SetHiddenInGame(false);
	AttackCollision->SetVisibility(false);
	AttackCollision->SetActive(false);

	DebugAttackCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("DebugAttackCollision"));
	DebugAttackCollision->AttachTo(RootComponent);
	DebugAttackCollision->SetRelativeLocation(FVector((Capsule->GetScaledCapsuleRadius() + SimpleAttack.AttackLength) / 2.0f, 0.0f, 20.0f));
	DebugAttackCollision->SetBoxExtent(FVector(SimpleAttack.AttackLength - Capsule->GetScaledCapsuleRadius(), SimpleAttack.AttackWidth, 32.0f));
	DebugAttackCollision->SetGenerateOverlapEvents(false);
	DebugAttackCollision->SetHiddenInGame(false);
	DebugAttackCollision->SetVisibility(false);
	DebugAttackCollision->SetActive(false);

	MovePhase = EEnemyPhase::None;
	AttackPhase = EAttackPhase::NotAttacking;
	AttackTarget = nullptr;

	UnstuckPlayerSightUpdate = 0.2f;
	UnstuckAroundPlayerRadius = 1000.0f;
	MaxAttempsToUnstuck = 10;

	DebugDamageIndicator = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Debug Damage Indicator"));
	DebugDamageIndicator->SetupAttachment(RootComponent);
	DebugDamageIndicator->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
	DebugDamageIndicator->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DebugDamageIndicator->SetGenerateOverlapEvents(false);
	DebugDamageIndicator->SetVisibility(false);

	DebugDamageIndicatorTime = 3.0f;

	bIsDebugOn = false;
}

void ABaseNPCSimpleChase::BeginPlay()
{
	GetWorld()->GetTimerManager().SetTimer(DelayedInitTimerHandle, this, &ABaseNPCSimpleChase::DelayedInit, DelayedInitTime, false);
	Super::BeginPlay();
}

void ABaseNPCSimpleChase::DelayedInit()
{
	AttackTarget = Cast<AHypercubeCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	NoticeCollision->SetGenerateOverlapEvents(true);
	TickSemaphore = 0;
	SetActorTickEnabled(false);

	if (!LevelController)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseLevelController::StaticClass(), FoundActors);
		if (FoundActors.Num())
		{
			LevelController = Cast<ABaseLevelController>(FoundActors[0]);
			if (LevelController)
			{
				LevelController->AddEnemy(this);
			}
		}
	}
}

void ABaseNPCSimpleChase::SetTickState(bool bToActivate)
{
	if (bToActivate)
	{
		if (++TickSemaphore == 1)
		{
			SetActorTickEnabled(true);
		}
	}
	else
	{
		if (!TickSemaphore)
		{
			return;
		}
		if (--TickSemaphore == 0)
		{
			SetActorTickEnabled(false);
		}
	}
}

void ABaseNPCSimpleChase::ForceTickDisable()
{
	TickSemaphore = 0;
	SetActorTickEnabled(false);
}

void ABaseNPCSimpleChase::Tick(float DeltaSeconds)
{
	if (AttackPhase != EAttackPhase::NotAttacking)
	{
		TickRotateToTarget(DeltaSeconds);
		if (AttackPhase == EAttackPhase::Attacking)
		{
			CheckPlayerHit();
			TickMoveForward(DeltaSeconds);
		}
	}
	if (MovePhase == EEnemyPhase::Noticing)
	{
		TickRotateToTarget(DeltaSeconds);
	}
	Super::Tick(DeltaSeconds);
}

void ABaseNPCSimpleChase::TickRotateToTarget(float DeltaSeconds)
{
	FVector ToTarget = AttackTarget->GetActorLocation() - GetActorLocation();
	ToTarget.Z = 0.0f;
	ToTarget.Normalize();
	SetActorRotation(UKismetMathLibrary::MakeRotFromXZ(FMath::Lerp(GetActorForwardVector(), ToTarget, DeltaSeconds * SimpleAttack.AttackRotationMultiplier), FVector::ZAxisVector));
}

void ABaseNPCSimpleChase::TickMoveForward(float DeltaSeconds)
{
	AddActorWorldOffset(GetActorForwardVector() * SimpleAttack.AttackMoveForwardSpeed * DeltaSeconds, true);
}

void ABaseNPCSimpleChase::CheckPlayerHit()
{
	TSet<AActor*> collisions;
	AttackCollision->GetOverlappingActors(collisions, AHypercubeCharacter::StaticClass());
	if (collisions.Num())
	{
		AttackTarget->TakeDamage(SimpleAttack.Damage);
	}
}

void ABaseNPCSimpleChase::ActivateDebugDamageIndicator()
{
	DebugDamageIndicator->SetVisibility(true);
	if (GetWorld()->GetTimerManager().IsTimerActive(DebugDamageIndicatorTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(DebugDamageIndicatorTimerHandle);
	}
	GetWorld()->GetTimerManager().SetTimer(DebugDamageIndicatorTimerHandle, this, &ABaseNPCSimpleChase::OnEndDebugDamageIndicatorTimer, DebugDamageIndicatorTime, false);
}

void ABaseNPCSimpleChase::OnEndDebugDamageIndicatorTimer()
{
	DebugDamageIndicator->SetVisibility(false);
}

void ABaseNPCSimpleChase::SetAttackCollision(bool bToActivate)
{
	if (bIsDebugOn)
	{
		AttackCollision->SetVisibility(bToActivate);
	}
	AttackCollision->SetActive(bToActivate);
}

void ABaseNPCSimpleChase::SetDebugAttackCollision(bool bToActivate)
{
	if (bIsDebugOn)
	{
		DebugAttackCollision->SetVisibility(bToActivate);
		DebugAttackCollision->SetActive(bToActivate);
	}
}

void ABaseNPCSimpleChase::TakeDamage(float Damage)
{
	Health -= Damage;
	if (bIsDebugOn)
	{
		ActivateDebugDamageIndicator();
	}
	EnemyActionDelegate.Broadcast(EEnemyAction::Damaged, true);
	if (Health <= 0.0f)
	{
		PlayDeath();
	}
}

void ABaseNPCSimpleChase::OnNotice()
{
	AttackTarget->OnEnemyAggro(this);
	MovePhase = EEnemyPhase::Noticing;
	SetTickState(true);
	GetWorld()->GetTimerManager().SetTimer(NoticeTimerHandle, this, &ABaseNPCSimpleChase::AfterNotice, AggroTime, false);
}

void ABaseNPCSimpleChase::AfterNotice()
{
	MovePhase = EEnemyPhase::Chasing;
	SetTickState(false);
}

void ABaseNPCSimpleChase::Attack()
{
	switch (AttackPhase)
	{
	case EAttackPhase::NotAttacking:
		UE_LOG(LogTemp, Warning, TEXT("Enemy Attacks!"));
		AttackPhase = EAttackPhase::Opener;
		SetTickState(true);
		SetDebugAttackCollision(true);
		GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &ABaseNPCSimpleChase::Attack, SimpleAttack.OpenerTime, false);
		break;
	case EAttackPhase::Opener:
		AttackPhase = EAttackPhase::Attacking;
		SetDebugAttackCollision(false);
		SetAttackCollision(true);
		GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &ABaseNPCSimpleChase::Attack, SimpleAttack.AttackTime, false);
		break;
	case EAttackPhase::Attacking:
		AttackPhase = EAttackPhase::AfterAttack;
		SetAttackCollision(false);
		SetDebugAttackCollision(true);
		GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &ABaseNPCSimpleChase::Attack, SimpleAttack.AfterAttackTime, false);
		break;
	case EAttackPhase::AfterAttack:
		AttackPhase = EAttackPhase::NotAttacking;
		SetDebugAttackCollision(false);
		SetTickState(false);
		EnemyActionDelegate.Broadcast(EEnemyAction::AttackEnd, true);
		break;
	default:
		break;
	}
}

void ABaseNPCSimpleChase::JumpTo(FVector Destination)
{
	UE_LOG(LogTemp, Warning, TEXT("%f, %f, %f"), Destination.X, Destination.Y, Destination.Z);
	FVector NowPos = GetActorLocation();
	FVector LookDestination = Destination - NowPos;
	LookDestination.Z = 0.0f;
	SetActorRotation(LookDestination.Rotation().Quaternion());
	FVector Velocity;
	Velocity.X = (Destination.X - NowPos.X) / JumpTime;
	Velocity.Y = (Destination.Y - NowPos.Y) / JumpTime;
	Velocity.Z = Destination.Z - NowPos.Z - 0.25f * JumpTime * JumpTime * MoveComp->GetGravityZ();
	LaunchCharacter(Velocity, true, true);
	UE_LOG(LogTemp, Warning, TEXT("%f"), MoveComp->GetGravityZ());
	GetWorld()->GetTimerManager().SetTimer(JumpTimerHandle, this, &ABaseNPCSimpleChase::OnEndJump, JumpTime, false);
}

void ABaseNPCSimpleChase::OnEndJump()
{
	UE_LOG(LogTemp, Warning, TEXT("EndJump!"));
	EnemyActionDelegate.Broadcast(EEnemyAction::JumpEnd, true);
}

void ABaseNPCSimpleChase::PlayDeath()
{
	AttackTarget->OnEnemyDeath(this);
	EnemyDeathDelegate.Broadcast();
}

void ABaseNPCSimpleChase::SetSlowDebuff(float Mult, float Time)
{
	if (GetWorld()->GetTimerManager().IsTimerActive(SlowDebuffTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(SlowDebuffTimerHandle);
	}
	else
	{
		BaseSpeed = MoveComp->MaxWalkSpeed;

		MoveComp->MaxWalkSpeed *= Mult;

		EnemyActionDelegate.Broadcast(EEnemyAction::SlowDebuff, true);
	}
	GetWorld()->GetTimerManager().SetTimer(SlowDebuffTimerHandle, this, &ABaseNPCSimpleChase::OnEndSlowDebuff, Time, false);
}

void ABaseNPCSimpleChase::OnEndSlowDebuff()
{
	MoveComp->MaxWalkSpeed = BaseSpeed;
	EnemyActionDelegate.Broadcast(EEnemyAction::SlowDebuffEnd, true);
}

void ABaseNPCSimpleChase::SetDamageDebuff(float Mult, float Time)
{
	if (GetWorld()->GetTimerManager().IsTimerActive(DamageDebuffTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(DamageDebuffTimerHandle);
	}
	else
	{
		BaseDamage = SimpleAttack.Damage;

		SimpleAttack.Damage *= Mult;

		EnemyActionDelegate.Broadcast(EEnemyAction::DamageDecreaseDebuff, true);
	}
	GetWorld()->GetTimerManager().SetTimer(DamageDebuffTimerHandle, this, &ABaseNPCSimpleChase::OnEndDamageDebuff, Time, false);
}

void ABaseNPCSimpleChase::OnEndDamageDebuff()
{
	SimpleAttack.Damage = BaseDamage;
	EnemyActionDelegate.Broadcast(EEnemyAction::DamageDecreaseDebuffEnd, true);
}

bool ABaseNPCSimpleChase::PlayerHasSightOn() const
{
	if (!AttackTarget->GetController()->LineOfSightTo(this))
	{
		return false;
	}
	FVector2D ScreenLocation;
	if (!UGameplayStatics::ProjectWorldToScreen(UGameplayStatics::GetPlayerController(GetWorld(), 0), GetActorLocation(), ScreenLocation))
	{
		return false;
	}
	const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
	return ScreenLocation.X > 0 && ScreenLocation.Y > 0 && ScreenLocation.X < ViewportSize.X && ScreenLocation.Y < ViewportSize.Y;
}

void ABaseNPCSimpleChase::Unstuck()
{
	if (PlayerHasSightOn())
	{
		GetWorld()->GetTimerManager().SetTimer(CheckPlayerSightTimerHandle, this, &ABaseNPCSimpleChase::Unstuck, UnstuckPlayerSightUpdate, false);
		return;
	}
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARecastNavMesh::StaticClass(), FoundActors);
	class ARecastNavMesh* NavData = Cast<ARecastNavMesh>(FoundActors[0]);
	FNavLocation FindResult;
	FVector PlayerLocation = AttackTarget->GetActorLocation();
	for (int i = 0; i < MaxAttempsToUnstuck; ++i)
	{
		if (NavData->GetRandomReachablePointInRadius(PlayerLocation, UnstuckAroundPlayerRadius, FindResult))
		{
			SetActorLocation(FindResult.Location, false, nullptr, ETeleportType::ResetPhysics);
			if (!PlayerHasSightOn())
			{
				EnemyActionDelegate.Broadcast(EEnemyAction::UnstuckEnd, true);
				return;
			}
		}
	}
	EnemyActionDelegate.Broadcast(EEnemyAction::UnstuckEnd, false);
}
