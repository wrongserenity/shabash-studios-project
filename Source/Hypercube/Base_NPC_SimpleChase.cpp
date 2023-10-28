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
	bUseControllerRotationYaw = false;

	Capsule = GetCapsuleComponent();
	Capsule->InitCapsuleSize(42.0f, 96.0f);

	MoveComp = GetCharacterMovement();
	MoveComp->JumpZVelocity = 560.0f;
	MoveComp->bOrientRotationToMovement = true;

	AttackCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackCollision"));
	AttackCollision->AttachTo(RootComponent);
	AttackCollision->SetRelativeLocation(FVector(80.0f, 0.0f, 20.0f));
	AttackCollision->SetWorldScale3D(FVector(1.8f, 1.0f, 1.0f));
	AttackCollision->SetGenerateOverlapEvents(true);
	AttackCollision->SetHiddenInGame(false);
	AttackCollision->SetVisibility(false);
	AttackCollision->SetActive(false);

	SimpleAttack = { 25.0f, 0.7f, 0.3f, 0.2f, 7.5f, 150.0f };

	//Damage = 25.0f;
	//OpenerTime = 0.7f;
	//AttackTime = 0.3f;
	//AfterAttackTime = 0.2f;
	//AttackRotationMultiplier = 7.5f;
	//AttackMoveForwardSpeed = 50.0f;

	Phase = EAttackPhase::NotAttacking;
	AttackTarget = nullptr;
}

// Called when the game starts or when spawned
void ABase_NPC_SimpleChase::BeginPlay()
{
	Super::BeginPlay();
	
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
	SetActorRotation(UKismetMathLibrary::MakeRotFromX(FMath::Lerp(GetActorForwardVector(), ToTarget, DeltaSeconds * SimpleAttack.AttackRotationMultiplier)));
}

void ABase_NPC_SimpleChase::TickMoveForward(float DeltaSeconds)
{
	AddActorWorldOffset(GetActorForwardVector() * SimpleAttack.AttackMoveForwardSpeed * DeltaSeconds);
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
	case EAttackPhase::NotAttacking:
		Phase = EAttackPhase::Opener;
		GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &ABase_NPC_SimpleChase::Attack, SimpleAttack.OpenerTime, false);
		return;
	case EAttackPhase::Opener:
		Phase = EAttackPhase::Attacking;
		SetAttackCollision(true);
		GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &ABase_NPC_SimpleChase::Attack, SimpleAttack.AttackTime, false);
		return;
	case EAttackPhase::Attacking:
		Phase = EAttackPhase::AfterAttack;
		SetAttackCollision(false);
		GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &ABase_NPC_SimpleChase::Attack, SimpleAttack.AfterAttackTime, false);
		return;
	case EAttackPhase::AfterAttack:
		Phase = EAttackPhase::NotAttacking;
		AttackEndDelegate.Broadcast(true);
	}
}
