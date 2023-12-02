#include "Base_EnemySpawnPoint.h"
#include "Base_NPC_SimpleChase.h"
#include "Components/CapsuleComponent.h"


ABase_EnemySpawnPoint::ABase_EnemySpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;
	EnemyClass = ABase_NPC_SimpleChase::StaticClass();
	EnemySpawnHeight = Cast<ABase_NPC_SimpleChase>(EnemyClass->GetDefaultObject())->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

class ABase_NPC_SimpleChase* ABase_EnemySpawnPoint::SpawnEnemy() const
{
	FVector SpawnLocation = GetActorLocation();
	SpawnLocation.Z += EnemySpawnHeight;
	FRotator SpawnRotation = GetActorRotation();
	AActor* SpawnedActor = GetWorld()->SpawnActor(EnemyClass, &SpawnLocation, &SpawnRotation);
	if (SpawnedActor)
	{
		ABase_NPC_SimpleChase* Enemy = Cast<ABase_NPC_SimpleChase>(SpawnedActor);
		if (Enemy)
		{
			Enemy->SpawnDefaultController();
			return Enemy;
		}
	}
	return nullptr;
}
