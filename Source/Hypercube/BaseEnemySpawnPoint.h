#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseNPCSimpleChase.h"
#include "BaseEnemySpawnPoint.generated.h"

// Base class for enemy spawn point

UCLASS()
class HYPERCUBE_API ABaseEnemySpawnPoint : public AActor
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EnemyParameters, meta = (AllowPrivateAccess = "true"))
	class UClass* EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EnemyParameters, meta = (AllowPrivateAccess = "true"))
	float EnemySpawnHeight;

public:

	ABaseEnemySpawnPoint();

	UFUNCTION(BlueprintCallable)
	class ABaseNPCSimpleChase* SpawnEnemy(int Level = 0, EEnemyLevelingType LevelingType = EEnemyLevelingType::None) const;

};
