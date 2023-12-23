#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseEnemySpawnPoint.generated.h"

UCLASS()
class HYPERCUBE_API ABaseEnemySpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	ABaseEnemySpawnPoint();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EnemyParameters)
	class UClass* EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EnemyParameters)
	float EnemySpawnHeight;

	UFUNCTION(BlueprintCallable)
	class ABaseNPCSimpleChase* SpawnEnemy() const;

};
