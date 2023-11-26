#include "Base_LevelController.h"
#include "Kismet/GameplayStatics.h"
#include "HypercubeCharacter.h"
#include "Base_NPC_SimpleChase.h"
#include "Base_EnemySpawnPoint.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"

ABase_LevelController::ABase_LevelController()
{
	PrimaryActorTick.bCanEverTick = true;

	AfterPlayerDeathTime = AfterAllEnemiesDeadTime = 5.0f;

	NoticeSoundTurnOffTime = 1.0f;
	bEnemyCanNoticeSound = true;

	FootstepSoundTurnOffTime = 0.2f;
	bEnemyCanFootstepSound = true;

	LevelNames = { TEXT("training"), TEXT("level_1_upd"), TEXT("level_2"), TEXT("level_3") };

	SaveSlotName = "RunDataSaveSlot";
	CurLevelData = { false, 0.0f, 0.0f, 0, 1.0f, 1.0f, 0, 0.0f };

	EnemiesKilled = 0;

	DifficultyParameter = 0.5f;

	DeathCountBounds = { 1, 3, 5, 10 };
	DeathCountValues = { 1.0f, 0.7f, 0.5f, 0.3f };
	DeathCountCost = 0.4f;

	OnDeathEnemyAggroBounds = { 20, 15, 10, 5 };
	OnDeathEnemyAggroValues = { 1.0f, 0.7f, 0.5f, 0.3f };
	OnDeathEnemyAggroCost = 0.4f;

	PlayTimeBounds = { 600.0f, 300.0f, 60.0f };
	PlayTimeValues = { 0.7f, 0.5f, 0.3f };
	PlayTimeCost = 0.2f;

	DifficultyParameterBounds = { 0.0f, 0.15f, 0.35f, 0.5f, 0.65f, 0.85f, 1.0f };

	PlayerVelocityValues = { 1.1f, 1.0f, 1.0f, 1.0f, 1.0f, 0.9f, 0.9f };
	PlayerDamageMultiplerValues = { 1.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.5f };
	PlayerVampirismValues = { 0.05f, 0.02f, 0.02f, 0.02f, 0.0f, 0.0f, 0.0f };

	EnemyVelocityValues = { 0.8f, 0.8f, 0.8f, 1.0f, 1.0f, 1.0f, 1.2f };
	EnemyDamageValues = { 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.5f, 1.5f };
	EnemyNoticeRadiusValues = { 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.5f };
	EnemyCountPercentageValues = { 0.3f, 0.3f, 0.7f, 0.7f, 1.0f, 1.0f, 1.0f };

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	MusicComp_Low = CreateDefaultSubobject<UAudioComponent>(TEXT("Music Low"));
	MusicComp_Low->SetupAttachment(RootComponent);

	MusicComp_High = CreateDefaultSubobject<UAudioComponent>(TEXT("Music High"));
	MusicComp_High->SetupAttachment(RootComponent);

	MusicParameter = TargetMusicParameter = 0.0f;
	MusicChangeSpeed = 2.0f;
	MusicRefreshFrequency = 2.0f;
	MusicVolumeMultiplier = 0.25f;
	MusicRefreshTimer = 0.0f;
}

void ABase_LevelController::BeginPlay()
{
	LoadLevelData();
	DifficultyParameter = GetDifficultyParameter();
	SpawnEnemies();
	MusicComp_Low->SetVolumeMultiplier(((MusicParameter < 0.5f ? MusicParameter * 2.0f : 1.0f) + 0.001f) * MusicVolumeMultiplier);
	MusicComp_High->SetVolumeMultiplier(((MusicParameter < 0.5f ? 0.0f : (MusicParameter - 0.5f) * 2.0f) + 0.001f) * MusicVolumeMultiplier);
	MusicComp_Low->Play();
	MusicComp_High->Play();
	Super::BeginPlay();
}

void ABase_LevelController::Tick(float DeltaSeconds)
{
	MusicRefreshTimer += DeltaSeconds;
	if (MusicRefreshTimer >= MusicRefreshFrequency)
	{
		MusicRefreshTimer = 0.0f;
		TargetMusicParameter = GetTargetMusicParameter();
	}
	if (MusicParameter != TargetMusicParameter)
	{
		MusicParameter = FMath::Lerp(MusicParameter, TargetMusicParameter, MusicChangeSpeed * DeltaSeconds);
		if (FMath::IsNearlyEqual(MusicParameter, TargetMusicParameter, 0.001f))
		{
			MusicParameter = TargetMusicParameter;
		}
		MusicComp_Low->SetVolumeMultiplier(((MusicParameter < 0.5f ? MusicParameter * 2.0f : 1.0f) + 0.001f) * MusicVolumeMultiplier);
		MusicComp_High->SetVolumeMultiplier(((MusicParameter < 0.5f ? 0.0f : (MusicParameter - 0.5f) * 2.0f) + 0.001f) * MusicVolumeMultiplier);
	}
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
	float EnemyPercentage = GetOutputParameterFrom(DifficultyParameter, DifficultyParameterBounds, EnemyCountPercentageValues);
	BeginEnemyCount = FMath::CeilToInt(float(SpawnPoints.Num()) * EnemyPercentage);
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
		SetEnemyParams(Enemy);
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
	SetPlayerParams();
}

void ABase_LevelController::OnPlayerDeath()
{
	CurLevelData.PlayerWon = false;
	SaveLevelData();
	GetWorld()->GetTimerManager().SetTimer(AfterLevelTimerHandle, this, &ABase_LevelController::AfterPlayerDeath, AfterPlayerDeathTime, false);
}

void ABase_LevelController::AfterPlayerDeath()
{
	LoadNewLevel();
}

void ABase_LevelController::OnAllEnemiesDead()
{
	CurLevelData.PlayerWon = true;
	SaveLevelData();
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
	UpdateMaxMultiplicator(Player->DamageMultiplier);
	CurLevelData.OnDeathMultiplicator = Player->DamageMultiplier;
	CurLevelData.OnDeathEnemyChasing = Player->GetEnemyChasingCount();
	CurLevelData.PlayTime = UGameplayStatics::GetRealTimeSeconds(GetWorld());
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
	UGameplayStatics::OpenLevel(GetWorld(), LevelNames[1]);
}

void ABase_LevelController::ClearLevelData()
{
	LevelData.Empty();
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

void ABase_LevelController::SetFootstepSoundTurnOff()
{
	bEnemyCanFootstepSound = false;
	GetWorld()->GetTimerManager().SetTimer(FootstepSoundTurnOffTimerHandle, this, &ABase_LevelController::OnEndFootstepSoundTurnedOff, FootstepSoundTurnOffTime, false);
}

void ABase_LevelController::OnEndFootstepSoundTurnedOff()
{
	bEnemyCanFootstepSound = true;
}

float ABase_LevelController::GetDifficultyParameter()
{
	if (!FMath::IsNearlyEqual(DeathCountCost + OnDeathEnemyAggroCost + PlayTimeCost, 1.0f))
	{
		UE_LOG(LogTemp, Error, TEXT("Sum of input parameter costs must be equal to 1!"));
	}
	if (!LevelData.Num())
	{
		return 0.5f;
	}
	int i = LevelData.Num() - 1;
	while (i >= 0 && !LevelData[i].PlayerWon)
	{
		--i;
	}
	int DeathCount = LevelData.Num() - 1 - i;
	int OnDeathChasing = LevelData.Last().OnDeathEnemyChasing;
	float PlayTime = LevelData.Last().PlayTime;
	bool IsWon = LevelData.Last().PlayerWon;

	float DeathCountParameter = GetDifficultyParameterFrom(DeathCount, DeathCountBounds, DeathCountValues) * DeathCountCost;
	float OnDeathChasingParameter = (IsWon ? 1.0f : GetDifficultyParameterFrom(OnDeathChasing, OnDeathEnemyAggroBounds, OnDeathEnemyAggroValues)) * OnDeathEnemyAggroCost;
	float PlayTimeParameter = (IsWon ? 1.0f : GetDifficultyParameterFrom(PlayTime, PlayTimeBounds, PlayTimeValues)) * PlayTimeCost;

	UE_LOG(LogTemp, Warning, TEXT("In: %d, %d, %f"), DeathCount, OnDeathChasing, PlayTime);
	UE_LOG(LogTemp, Warning, TEXT("Out: %f, %f, %f"), DeathCountParameter, OnDeathChasingParameter, PlayTimeParameter);
	return DeathCountParameter + OnDeathChasingParameter + PlayTimeParameter;
}

void ABase_LevelController::SetPlayerParams()
{
	Player->GetCharacterMovement()->MaxWalkSpeed *= GetOutputParameterFrom(DifficultyParameter, DifficultyParameterBounds, PlayerVelocityValues);
	Player->DamageMultiplierEnemyCost *= GetOutputParameterFrom(DifficultyParameter, DifficultyParameterBounds, PlayerDamageMultiplerValues);
	Player->Vampirism = GetOutputParameterFrom(DifficultyParameter, DifficultyParameterBounds, PlayerVampirismValues);
}

void ABase_LevelController::SetEnemyParams(class ABase_NPC_SimpleChase* Enemy)
{
	Enemy->GetCharacterMovement()->MaxWalkSpeed *= GetOutputParameterFrom(DifficultyParameter, DifficultyParameterBounds, EnemyVelocityValues);
	Enemy->SimpleAttack.Damage *= GetOutputParameterFrom(DifficultyParameter, DifficultyParameterBounds, EnemyDamageValues);
	Enemy->GetNoticeCollision()->SetSphereRadius(Enemy->AggroRadius * GetOutputParameterFrom(DifficultyParameter, DifficultyParameterBounds, EnemyNoticeRadiusValues));
}

float ABase_LevelController::GetTargetMusicParameter()
{
	int ChasingCount = Player->GetEnemyChasingCount();
	if (!ChasingCount)
	{
		return 0.0f;
	}
	if (ChasingCount < 10)
	{
		return 0.5f;
	}
	return 1.0f;
}