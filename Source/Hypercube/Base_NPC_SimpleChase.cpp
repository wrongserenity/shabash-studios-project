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

	Capsule = GetCapsuleComponent();
	Capsule->InitCapsuleSize(42.0f, 96.0f);

	MoveComp = GetCharacterMovement();
	MoveComp->JumpZVelocity = 560.0f;

	AttackCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackCollision"));
	AttackCollision->AttachTo(RootComponent);
	AttackCollision->SetRelativeLocation(FVector(80.0f, 0.0f, 20.0f));
	AttackCollision->SetWorldScale3D(FVector(1.8f, 1.0f, 1.0f));
	AttackCollision->SetGenerateOverlapEvents(true);
	AttackCollision->SetHiddenInGame(false);
	AttackCollision->SetVisibility(false);
	AttackCollision->SetActive(false);

	Damage = 25.0f;
	OpenerTime = 0.7f;
	AttackTime = 0.3f;
	AfterAttackTime = 0.2f;
	AttackRotationMultiplier = 7.5f;
	Phase = AttackPhase::NotAttacking;
	AttackTarget = nullptr;
}

// Called when the game starts or when spawned
void ABase_NPC_SimpleChase::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABase_NPC_SimpleChase::Tick(float DeltaSeconds)
{
	if (Phase != AttackPhase::NotAttacking)
	{
		TickRotateToTarget(DeltaSeconds);
	}
	if (Phase == AttackPhase::Attacking)
	{
		TSet<AActor*> collisions;
		AttackCollision->GetOverlappingActors(collisions, AHypercubeCharacter::StaticClass());
		if (collisions.Num())
		{
			AttackTarget->TakeDamage(Damage);
		}
	}
	Super::Tick(DeltaSeconds);
}

void ABase_NPC_SimpleChase::TickRotateToTarget(float DeltaSeconds)
{
	FVector ToTarget = AttackTarget->GetActorLocation() - GetActorLocation();
	ToTarget.Z = 0.0f;
	ToTarget.Normalize();
	SetActorRotation(UKismetMathLibrary::MakeRotFromX(FMath::Lerp(GetActorForwardVector(), ToTarget, DeltaSeconds * AttackRotationMultiplier)));
}

void ABase_NPC_SimpleChase::SetAttackCollision(bool Active)
{
	AttackCollision->SetVisibility(Active);
	AttackCollision->SetActive(Active);
}

void ABase_NPC_SimpleChase::Attack()
{
	if (!AttackTarget)
	{
		AttackTarget = Cast<AHypercubeCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	}
	switch (Phase)
	{
	case AttackPhase::NotAttacking:
		Phase = AttackPhase::Opener;
		GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &ABase_NPC_SimpleChase::Attack, OpenerTime, false);
		return;
	case AttackPhase::Opener:
		Phase = AttackPhase::Attacking;
		SetAttackCollision(true);
		GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &ABase_NPC_SimpleChase::Attack, AttackTime, false);
		return;
	case AttackPhase::Attacking:
		Phase = AttackPhase::AfterAttack;
		SetAttackCollision(false);
		GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &ABase_NPC_SimpleChase::Attack, AfterAttackTime, false);
		return;
	case AttackPhase::AfterAttack:
		Phase = AttackPhase::NotAttacking;
		AttackEndDelegate.Broadcast(true);
	}
}
