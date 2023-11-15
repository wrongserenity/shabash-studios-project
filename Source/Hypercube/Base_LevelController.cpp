#include "Base_LevelController.h"
#include "Kismet/GameplayStatics.h"
#include "HypercubeCharacter.h"
#include "Base_NPC_SimpleChase.h"

ABase_LevelController::ABase_LevelController()
{
	PrimaryActorTick.bCanEverTick = false;

	AfterPlayerDeathTime = AfterAllEnemiesDeadTime = 5.0f;

	NextLevelName = TEXT("level1");

	SaveSlotName = "RunDataSaveSlot";
	CurLevelData = { false, 0.0f, 0, 1.0f, 1.0f };
}

void ABase_LevelController::BeginPlay()
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
	Super::BeginPlay();
}

void ABase_LevelController::AddEnemiesKilled()
{
	++CurLevelData.EnemiesKilled;
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
	CurLevelData.OnDeathMultiplicator = Player->DamageMulptiplier;
	UpdateMaxMultiplicator(Player->DamageMulptiplier);
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

float ABase_LevelController::GetPlayerHealthValue()
{
	if (!LevelData.Num())
	{
		return 150.0f;
	}
	float NewHealth = 150.0f - LevelData.Last().Score / 16.0f;
	NewHealth = NewHealth < 40.0f ? 40.0f : NewHealth;
	return NewHealth;
}