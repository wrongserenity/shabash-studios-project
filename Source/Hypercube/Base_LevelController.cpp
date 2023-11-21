#include "Base_LevelController.h"
#include "Kismet/GameplayStatics.h"
#include "HypercubeCharacter.h"
#include "Base_NPC_SimpleChase.h"
#include "Base_EnemySpawnPoint.h"

ABase_LevelController::ABase_LevelController()
{
	PrimaryActorTick.bCanEverTick = false;

	AfterPlayerDeathTime = AfterAllEnemiesDeadTime = 5.0f;

	NoticeSoundTurnOffTime = 1.0f;

	bEnemyCanNoticeSound = true;

	NextLevelName = TEXT("level1");

	SaveSlotName = "RunDataSaveSlot";
	CurLevelData = { false, 0.0f, 0.0f, 0, 1.0f, 1.0f };

	EnemiesKilled = 0;
}

void ABase_LevelController::BeginPlay()
{
	LoadLevelData();
	SpawnEnemies();
	Super::BeginPlay();
}

void ABase_LevelController::LoadLevelData()
{
	UBase_RunDataSave* LoadedData = Cast<UBase_RunDataSave>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
	if (LoadedData)
	{
		LevelData = LoadedData->LevelDataArr;
		UE_LOG(LogTemp, Warning, TEXT("Total level walkthroughs: %d"), LevelData.Num());
		if (LevelData.Num())
		{
			LevelData.Last().Log();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No data to load"));
	}
}

void ABase_LevelController::SpawnEnemies()
{
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABase_EnemySpawnPoint::StaticClass(), SpawnPoints);
	for (int i = 0; i < SpawnPoints.Num(); ++i)
	{
		SpawnPoints.Swap(i, FMath::RandRange(i, SpawnPoints.Num() - 1));
	}
	BeginEnemyCount = FMath::CeilToInt(float(SpawnPoints.Num()) * GetEnemyPercentage());
	BeginEnemyCount = BeginEnemyCount > SpawnPoints.Num() ? SpawnPoints.Num() : BeginEnemyCount;
	CurLevelData.TotalEnemies = BeginEnemyCount;
	UE_LOG(LogTemp, Warning, TEXT("Enemies spawned: %d"), BeginEnemyCount);
	for (int i = 0; i < BeginEnemyCount; ++i)
	{
		ABase_EnemySpawnPoint* SpawnPoint = Cast<ABase_EnemySpawnPoint>(SpawnPoints[i]);
		if (SpawnPoint)
		{
			SpawnEnemy(SpawnPoint);
		}
	}
}

void ABase_LevelController::SpawnEnemy(class ABase_EnemySpawnPoint* SpawnPoint)
{
	ABase_NPC_SimpleChase* Enemy = SpawnPoint->SpawnEnemy();
	if (Enemy)
	{
		Enemy->LevelController = this;
		Enemies.Add(Enemy);
	}
}

void ABase_LevelController::AddEnemiesKilled()
{
	++EnemiesKilled;
}

void ABase_LevelController::AddEnemy(class ABase_NPC_SimpleChase* Enemy)
{
	Enemies.Add(Enemy);
}

void ABase_LevelController::RemoveEnemy(class ABase_NPC_SimpleChase* Enemy)
{
	if (Enemies.Contains(Enemy))
	{
		Enemies.Remove(Enemy);
		AddEnemiesKilled();
	}
	if (!Enemies.Num())
	{
		OnAllEnemiesDead();
	}
}

void ABase_LevelController::UpdateMaxMultiplicator(float NewMultiplicator)
{
	if (NewMultiplicator > CurLevelData.MaxMultiplicator)
	{
		CurLevelData.MaxMultiplicator = NewMultiplicator;
	}
}

void ABase_LevelController::SetPlayerCharacter(class AHypercubeCharacter* PlayerCharacter)
{
	Player = PlayerCharacter;
	Player->Health = Player->MaxHealth = GetPlayerHealthValue();
}

void ABase_LevelController::OnPlayerDeath()
{
	CurLevelData.PlayerWon = false;
	GetWorld()->GetTimerManager().SetTimer(AfterLevelTimerHandle, this, &ABase_LevelController::AfterPlayerDeath, AfterPlayerDeathTime, false);
}

void ABase_LevelController::AfterPlayerDeath()
{
	LoadNewLevel();
}

void ABase_LevelController::OnAllEnemiesDead()
{
	CurLevelData.PlayerWon = true;
	AllEnemiesDeadDelegate.Broadcast();
	GetWorld()->GetTimerManager().SetTimer(AfterLevelTimerHandle, this, &ABase_LevelController::AfterAllEnemiesDead, AfterAllEnemiesDeadTime, false);
}

void ABase_LevelController::AfterAllEnemiesDead()
{
	LoadNewLevel();
}

void ABase_LevelController::SaveLevelData()
{
	CurLevelData.Score = Player->Score;
	CurLevelData.EnemiesPercentageKilled = float(EnemiesKilled) / float(BeginEnemyCount);
	CurLevelData.OnDeathMultiplicator = Player->DamageMultiplier;
	UpdateMaxMultiplicator(Player->DamageMultiplier);
	LevelData.Add(CurLevelData);
	UBase_RunDataSave* SaveGameInstance = Cast<UBase_RunDataSave>(UGameplayStatics::CreateSaveGameObject(UBase_RunDataSave::StaticClass()));
	if (SaveGameInstance)
	{
		SaveGameInstance->LevelDataArr = LevelData;
		UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveSlotName, 0);
	}
}

void ABase_LevelController::LoadNewLevel()
{
	SaveLevelData();
	UGameplayStatics::OpenLevel(GetWorld(), NextLevelName);
}

void ABase_LevelController::ClearLevelData()
{
	LevelData.Empty();
}

float ABase_LevelController::GetPlayerHealthValue() const
{
	if (!LevelData.Num())
	{
		return 150.0f;
	}
	float NewHealth = 150.0f - LevelData.Last().Score / 16.0f;
	NewHealth = NewHealth < 40.0f ? 40.0f : NewHealth;
	return NewHealth;
}

float ABase_LevelController::GetEnemyPercentage() const
{
	if (!LevelData.Num())
	{
		return 0.8f;
	}
	float LastPercentage = LevelData.Last().EnemiesPercentageKilled;
	if (LastPercentage < 0.33f)
	{
		return 0.33f;
	}
	return LastPercentage;
}

void ABase_LevelController::SetNoticeSoundTurnOff()
{
	bEnemyCanNoticeSound = false;
	GetWorld()->GetTimerManager().SetTimer(NoticeSoundTurnOffTimerHandle, this, &ABase_LevelController::OnEndNoticeSoundTurnedOff, NoticeSoundTurnOffTime, false);
}

void ABase_LevelController::OnEndNoticeSoundTurnedOff()
{
	bEnemyCanNoticeSound = true;
}