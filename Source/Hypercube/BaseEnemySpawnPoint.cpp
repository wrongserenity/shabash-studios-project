#include "BaseEnemySpawnPoint.h"
#include "BaseNPCSimpleChase.h"
#include "Components/CapsuleComponent.h"

// Base class for enemy spawn point

ABaseEnemySpawnPoint::ABaseEnemySpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;
	EnemyClass = ABaseNPCSimpleChase::StaticClass();
	EnemySpawnHeight = Cast<ABaseNPCSimpleChase>(EnemyClass->GetDefaultObject())->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

class ABaseNPCSimpleChase* ABaseEnemySpawnPoint::SpawnEnemy() const
{
	FVector SpawnLocation = GetActorLocation();
	SpawnLocation.Z += EnemySpawnHeight;
	FRotator SpawnRotation = GetActorRotation();
	AActor* SpawnedActor = GetWorld()->SpawnActor(EnemyClass, &SpawnLocation, &SpawnRotation);
	if (SpawnedActor)
	{
		ABaseNPCSimpleChase* Enemy = Cast<ABaseNPCSimpleChase>(SpawnedActor);
		if (Enemy)
		{
			Enemy->SpawnDefaultController();
			return Enemy;
		}
	}
	return nullptr;
}
