// Fill out your copyright notice in the Description page of Project Settings.


#include "Base_NPC_SimpleChase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/BoxComponent.h"

// Sets default values
ABase_NPC_SimpleChase::ABase_NPC_SimpleChase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

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
}

// Called when the game starts or when spawned
void ABase_NPC_SimpleChase::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABase_NPC_SimpleChase::SetAttackCollision(bool active)
{
	AttackCollision->SetVisibility(active);
	AttackCollision->SetActive(active);
}
