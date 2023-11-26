// Fill out your copyright notice in the Description page of Project Settings.


#include "Base_NPC_SimpleChase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Math/UnrealMathVectorCommon.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "HypercubeCharacter.h"
#include "Components/SphereComponent.h"
#include "Base_LevelController.h"
#include "Components/WidgetComponent.h"

// Sets default values
ABase_NPC_SimpleChase::ABase_NPC_SimpleChase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
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

	SimpleAttack = { 25.0f, 0.7f, 0.3f, 0.2f, 7.5f, 150.0f, 75.0f, 35.0f };

	AttackCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackCollision"));
	AttackCollision->AttachTo(RootComponent);
	AttackCollision->SetRelativeLocation(FVector((Capsule->GetScaledCapsuleRadius() + SimpleAttack.AttackLength) / 2.0f, 0.0f, 20.0f));
	AttackCollision->SetBoxExtent(FVector(SimpleAttack.AttackLength - Capsule->GetScaledCapsuleRadius(), SimpleAttack.AttackWidth, 32.0f));
	AttackCollision->SetGenerateOverlapEvents(true);
	AttackCollision->SetHiddenInGame(false);
	AttackCollision->SetVisibility(false);
	AttackCollision->SetActive(false);

	Debug_AttackCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("DebugAttackCollision"));
	Debug_AttackCollision->AttachTo(RootComponent);
	Debug_AttackCollision->SetRelativeLocation(FVector((Capsule->GetScaledCapsuleRadius() + SimpleAttack.AttackLength) / 2.0f, 0.0f, 20.0f));
	Debug_AttackCollision->SetBoxExtent(FVector(SimpleAttack.AttackLength - Capsule->GetScaledCapsuleRadius(), SimpleAttack.AttackWidth, 32.0f));
	Debug_AttackCollision->SetGenerateOverlapEvents(false);
	Debug_AttackCollision->SetHiddenInGame(false);
	Debug_AttackCollision->SetVisibility(false);
	Debug_AttackCollision->SetActive(false);

	MovePhase = EEnemyPhase::None;
	AttackPhase = EAttackPhase::NotAttacking;
	AttackTarget = nullptr;

	Debug_DamageIndicator = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Debug Damage Indicator"));
	Debug_DamageIndicator->SetupAttachment(RootComponent);
	Debug_DamageIndicator->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
	Debug_DamageIndicator->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Debug_DamageIndicator->SetGenerateOverlapEvents(false);
	Debug_DamageIndicator->SetVisibility(false);

	Debug_DamageIndicatorTime = 3.0f;

	bDebug = false;

	SlowDebuffEffectWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Slow Debuff Effect"));
	SlowDebuffEffectWidget->SetupAttachment(RootComponent);
	SlowDebuffEffectWidget->SetRelativeLocation(FVector(0.0f, 0.0f, -Capsule->GetScaledCapsuleHalfHeight()));
	SlowDebuffEffectWidget->SetVisibility(false);
}

// Called when the game starts or when spawned
void ABase_NPC_SimpleChase::BeginPlay()
{
	GetWorld()->GetTimerManager().SetTimer(DelayedInitTimerHandle, this, &ABase_NPC_SimpleChase::DelayedInit, DelayedInitTime, false);
	Super::BeginPlay();
}

void ABase_NPC_SimpleChase::DelayedInit()
{
	AttackTarget = Cast<AHypercubeCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	NoticeCollision->SetGenerateOverlapEvents(true);
	TickSemaphore = 0;
	SetActorTickEnabled(false);

	if (!LevelController)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABase_LevelController::StaticClass(), FoundActors);
		if (FoundActors.Num())
		{
			LevelController = Cast<ABase_LevelController>(FoundActors[0]);
			LevelController->AddEnemy(this);
		}
	}
}

void ABase_NPC_SimpleChase::SetTickState(bool Activate)
{
	if (Activate)
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

void ABase_NPC_SimpleChase::ForceTickDisable()
{
	TickSemaphore = 0;
	SetActorTickEnabled(false);
}

void ABase_NPC_SimpleChase::Tick(float DeltaSeconds)
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

void ABase_NPC_SimpleChase::TickRotateToTarget(float DeltaSeconds)
{
	FVector ToTarget = AttackTarget->GetActorLocation() - GetActorLocation();
	ToTarget.Z = 0.0f;
	ToTarget.Normalize();
	SetActorRotation(UKismetMathLibrary::MakeRotFromXZ(FMath::Lerp(GetActorForwardVector(), ToTarget, DeltaSeconds * SimpleAttack.AttackRotationMultiplier), FVector::ZAxisVector));
}

void ABase_NPC_SimpleChase::TickMoveForward(float DeltaSeconds)
{
	AddActorWorldOffset(GetActorForwardVector() * SimpleAttack.AttackMoveForwardSpeed * DeltaSeconds, true);
}

void ABase_NPC_SimpleChase::CheckPlayerHit()
{
	TSet<AActor*> collisions;
	AttackCollision->GetOverlappingActors(collisions, AHypercubeCharacter::StaticClass());
	if (collisions.Num())
	{
		AttackTarget->TakeDamage(SimpleAttack.Damage);
	}
}

void ABase_NPC_SimpleChase::ActivateDebugDamageIndicator()
{
	Debug_DamageIndicator->SetVisibility(true);
	if (GetWorld()->GetTimerManager().IsTimerActive(Debug_DamageIndicatorTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(Debug_DamageIndicatorTimerHandle);
	}
	GetWorld()->GetTimerManager().SetTimer(Debug_DamageIndicatorTimerHandle, this, &ABase_NPC_SimpleChase::OnEndDebugDamageIndicatorTimer, Debug_DamageIndicatorTime, false);
}

void ABase_NPC_SimpleChase::OnEndDebugDamageIndicatorTimer()
{
	Debug_DamageIndicator->SetVisibility(false);
}

void ABase_NPC_SimpleChase::SetAttackCollision(bool Active)
{
	if (bDebug)
	{
		AttackCollision->SetVisibility(Active);
	}
	AttackCollision->SetActive(Active);
}

void ABase_NPC_SimpleChase::SetDebugAttackCollision(bool Active)
{
	if (bDebug)
	{
		Debug_AttackCollision->SetVisibility(Active);
		Debug_AttackCollision->SetActive(Active);
	}
}

void ABase_NPC_SimpleChase::TakeDamage(float Damage)
{
	Health -= Damage;
	if (bDebug)
	{
		ActivateDebugDamageIndicator();
	}
	EnemyDamagedDelegate.Broadcast();
	if (Health <= 0.0f)
	{
		PlayDeath();
	}
}

void ABase_NPC_SimpleChase::OnNotice()
{
	AttackTarget->OnEnemyAggro(this);
	MovePhase = EEnemyPhase::Noticing;
	SetTickState(true);
	GetWorld()->GetTimerManager().SetTimer(NoticeTimerHandle, this, &ABase_NPC_SimpleChase::AfterNotice, AggroTime, false);
}

void ABase_NPC_SimpleChase::AfterNotice()
{
	MovePhase = EEnemyPhase::Chasing;
	SetTickState(false);
}

void ABase_NPC_SimpleChase::Attack()
{
	switch (AttackPhase)
	{
	case EAttackPhase::NotAttacking:
		UE_LOG(LogTemp, Warning, TEXT("Enemy Attacks!"));
		AttackPhase = EAttackPhase::Opener;
		SetTickState(true);
		SetDebugAttackCollision(true);
		GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &ABase_NPC_SimpleChase::Attack, SimpleAttack.OpenerTime, false);
		break;
	case EAttackPhase::Opener:
		AttackPhase = EAttackPhase::Attacking;
		SetDebugAttackCollision(false);
		SetAttackCollision(true);
		GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &ABase_NPC_SimpleChase::Attack, SimpleAttack.AttackTime, false);
		break;
	case EAttackPhase::Attacking:
		AttackPhase = EAttackPhase::AfterAttack;
		SetAttackCollision(false);
		SetDebugAttackCollision(true);
		GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &ABase_NPC_SimpleChase::Attack, SimpleAttack.AfterAttackTime, false);
		break;
	case EAttackPhase::AfterAttack:
		AttackPhase = EAttackPhase::NotAttacking;
		SetDebugAttackCollision(false);
		SetTickState(false);
		AttackEndDelegate.Broadcast(true);
	}
}

void ABase_NPC_SimpleChase::JumpTo(FVector Destination)
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
	GetWorld()->GetTimerManager().SetTimer(JumpTimerHandle, this, &ABase_NPC_SimpleChase::OnEndJump, JumpTime, false);
}

void ABase_NPC_SimpleChase::OnEndJump()
{
	UE_LOG(LogTemp, Warning, TEXT("EndJump!"));
	JumpEndDelegate.Broadcast(true);
}

void ABase_NPC_SimpleChase::PlayDeath()
{
	AttackTarget->OnEnemyDeath(this);
	EnemyDeathDelegate.Broadcast();
}

class ABase_LevelController* ABase_NPC_SimpleChase::GetLevelController() const
{
	return LevelController;
}

class USphereComponent* ABase_NPC_SimpleChase::GetNoticeCollision() const
{
	return NoticeCollision;
}

void ABase_NPC_SimpleChase::SetSlowDebuff(float Mult, float Time)
{
	if (GetWorld()->GetTimerManager().IsTimerActive(SlowDebuffTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(SlowDebuffTimerHandle);
	}
	else
	{
		BaseSpeed = MoveComp->MaxWalkSpeed;

		MoveComp->MaxWalkSpeed *= Mult;

		SlowDebuffEffectWidget->SetVisibility(true);
	}
	GetWorld()->GetTimerManager().SetTimer(SlowDebuffTimerHandle, this, &ABase_NPC_SimpleChase::OnEndSlowDebuff, Time, false);
}

void ABase_NPC_SimpleChase::OnEndSlowDebuff()
{
	MoveComp->MaxWalkSpeed = BaseSpeed;
	SlowDebuffEffectWidget->SetVisibility(false);
}