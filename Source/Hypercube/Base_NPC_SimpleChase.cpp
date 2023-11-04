// Fill out your copyright notice in the Description page of Project Settings.


#include "Base_NPC_SimpleChase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Math/UnrealMathVectorCommon.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "HypercubeCharacter.h"

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

	SimpleAttack = { 25.0f, 0.7f, 0.3f, 0.2f, 7.5f, 150.0f, 75.0f, 35.0f };

	AttackCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackCollision"));
	AttackCollision->AttachTo(RootComponent);
	AttackCollision->SetRelativeLocation(FVector((Capsule->GetScaledCapsuleRadius() + SimpleAttack.AttackLength) / 2.0f, 0.0f, 20.0f));
	AttackCollision->SetBoxExtent(FVector(SimpleAttack.AttackLength - Capsule->GetScaledCapsuleRadius(), SimpleAttack.AttackWidth, 32.0f));
	AttackCollision->SetGenerateOverlapEvents(false);
	AttackCollision->SetHiddenInGame(false);
	AttackCollision->SetVisibility(false);
	AttackCollision->SetActive(false);

	Debug_AttackCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("DebugAttackCollision"));
	Debug_AttackCollision->AttachTo(RootComponent);
	Debug_AttackCollision->SetRelativeLocation(FVector((Capsule->GetScaledCapsuleRadius() + SimpleAttack.AttackLength) / 2.0f, 0.0f, 20.0f));
	Debug_AttackCollision->SetBoxExtent(FVector(SimpleAttack.AttackLength - Capsule->GetScaledCapsuleRadius(), SimpleAttack.AttackWidth, 32.0f));
	Debug_AttackCollision->SetGenerateOverlapEvents(true);
	Debug_AttackCollision->SetHiddenInGame(false);
	Debug_AttackCollision->SetVisibility(false);
	Debug_AttackCollision->SetActive(false);

	Phase = EAttackPhase::NotAttacking;
	AttackTarget = nullptr;

	Debug_DamageIndicator = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Debug Damage Indicator"));
	Debug_DamageIndicator->SetupAttachment(RootComponent);
	Debug_DamageIndicator->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
	Debug_DamageIndicator->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Debug_DamageIndicator->SetGenerateOverlapEvents(false);
	Debug_DamageIndicator->SetVisibility(false);

	Debug_DamageIndicatorTime = 3.0f;
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
	TickSemaphore = 0;
	SetActorTickEnabled(false);
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
	if (Phase != EAttackPhase::NotAttacking)
	{
		TickRotateToTarget(DeltaSeconds);
		if (Phase == EAttackPhase::Attacking)
		{
			CheckPlayerHit();
			TickMoveForward(DeltaSeconds);
		}
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
	AttackCollision->SetVisibility(Active);
	AttackCollision->SetActive(Active);
}

void ABase_NPC_SimpleChase::SetDebugAttackCollision(bool Active)
{
	Debug_AttackCollision->SetVisibility(Active);
	Debug_AttackCollision->SetActive(Active);
}

void ABase_NPC_SimpleChase::TakeDamage(float Damage)
{
	Health -= Damage;
	ActivateDebugDamageIndicator();
	if (Health <= 0.0f)
	{
		PlayDeath();
	}
}

void ABase_NPC_SimpleChase::OnNotice()
{
	AttackTarget->AddChasingDamageMultiplier(this);
}

void ABase_NPC_SimpleChase::Attack()
{
	switch (Phase)
	{
	case EAttackPhase::NotAttacking:
		Phase = EAttackPhase::Opener;
		SetTickState(true);
		SetDebugAttackCollision(true);
		GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &ABase_NPC_SimpleChase::Attack, SimpleAttack.OpenerTime, false);
		break;
	case EAttackPhase::Opener:
		Phase = EAttackPhase::Attacking;
		SetDebugAttackCollision(false);
		SetAttackCollision(true);
		GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &ABase_NPC_SimpleChase::Attack, SimpleAttack.AttackTime, false);
		break;
	case EAttackPhase::Attacking:
		Phase = EAttackPhase::AfterAttack;
		SetAttackCollision(false);
		SetDebugAttackCollision(true);
		GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &ABase_NPC_SimpleChase::Attack, SimpleAttack.AfterAttackTime, false);
		break;
	case EAttackPhase::AfterAttack:
		Phase = EAttackPhase::NotAttacking;
		SetDebugAttackCollision(false);
		SetTickState(false);
		AttackEndDelegate.Broadcast(true);
	}
}

void ABase_NPC_SimpleChase::PlayDeath()
{
	AttackTarget->RemoveChasingDamageMultiplier(this);
	Destroy();
}
