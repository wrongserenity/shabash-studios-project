#include "BaseLevelController.h"
#include "Kismet/GameplayStatics.h"
#include "HypercubeCharacter.h"
#include "BaseNPCSimpleChase.h"
#include "BaseEnemySpawnPoint.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "NiagaraComponent.h"

// Base class for level controller
// Level controller provides data saving and loading, enemy spawning and difficulty settings realisation

bool FScoreboardData::operator<(const FScoreboardData& Other) const
{
	return Score > Other.Score;
}

ABaseLevelController::ABaseLevelController()
{
	PrimaryActorTick.bCanEverTick = true;

	AfterPlayerDeathTime = 5.0f;
	FewEnemiesEventPercentage = 0.03f;

	NoticeSoundTurnOffTime = 1.0f;
	bEnemyCanNoticeSound = true;

	FootstepSoundTurnOffTime = 0.2f;
	bEnemyCanFootstepSound = true;

	DeathSoundTurnOffTime = 1.0f;
	bEnemyCanDeathSound = true;

	LevelNames = { "training", "level_1_upd", "level_2", "level_3" };
	LevelNamesToShow = { "Tutorial", "Level1", "Level2", "Level3" };

	SaveSlotName = "RunDataSaveSlot";
	CurLevelData = { false, 0.0f, 0.0f, 0, 1.0f, 1.0f, 0, 0.0f };

	EnemiesKilled = 0;

	DifficultyParameter = 0.5f;

	StackState = EEnemyStackState::None;

	SoftStackBeginEnemyCount = 35;
	SoftStackEndEnemyCount = 30;

	HardStackBeginEnemyCount = 45;
	HardStackEndEnemyCount = 40;

	EnemyStackQueryFrequency = 5.0f;

	bIsHardStackActive = false;

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
	EnemyLevelingPercentageValues = { 0.35f, 0.35f, 0.35f, 0.5f, 0.5f, 0.65f, 0.65f };

	ChainDamageMultiplier = 0.25f;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	MusicCompExplore = CreateDefaultSubobject<UAudioComponent>(TEXT("Music Exploration"));
	MusicCompExplore->SetupAttachment(RootComponent);

	MusicCompLow = CreateDefaultSubobject<UAudioComponent>(TEXT("Music Low"));
	MusicCompLow->SetupAttachment(RootComponent);

	MusicCompHigh = CreateDefaultSubobject<UAudioComponent>(TEXT("Music High"));
	MusicCompHigh->SetupAttachment(RootComponent);

	MusicParameter = TargetMusicParameter = 0.0f;
	MusicChangeSpeed = 2.0f;
	MusicRefreshFrequency = 2.0f;
	MusicVolumeMultiplier = 0.25f;
	MusicRefreshTimer = 0.0f;

	ChainBoostNiagaraAsset = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Chain Boost Niagara Asset"));
	ChainBoostNiagaraAsset->SetupAttachment(RootComponent);
	ChainBoostNiagaraAsset->SetAutoActivate(false);
	ChainBoostNiagaraAsset->SetActive(false);

	ChainBoostDamageNiagaraAsset = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Chain Boost Damage Niagara Asset"));
	ChainBoostDamageNiagaraAsset->SetupAttachment(RootComponent);
	ChainBoostDamageNiagaraAsset->SetAutoActivate(false);
	ChainBoostDamageNiagaraAsset->SetActive(false);
}

void ABaseLevelController::BeginPlay()
{
	UE_LOG(LogTemp, Warning, TEXT("%s"), *(GetWorld()->GetMapName()));
	CurLevelIndex = GetCurMapIndex();
	if (CurLevelIndex < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid map!"));
	}
	LoadLevelData();
	DifficultyParameter = GetDifficultyParameter();
	EnemyLevelingPercentage = GetOutputParameterFrom(DifficultyParameter, DifficultyParameterBounds, EnemyLevelingPercentageValues);
	SpawnEnemies();

	MusicCompExplore->SetVolumeMultiplier(((MusicParameter > 0.5f ? 0.0f : 1.0f - MusicParameter * 2.0f) + 0.001f) * MusicVolumeMultiplier);
	MusicCompLow->SetVolumeMultiplier(((MusicParameter < 0.5f ? MusicParameter * 2.0f : 1.0f) + 0.001f) * MusicVolumeMultiplier);
	MusicCompHigh->SetVolumeMultiplier(((MusicParameter < 0.5f ? 0.0f : (MusicParameter - 0.5f) * 2.0f) + 0.001f) * MusicVolumeMultiplier);
	MusicCompExplore->Play();
	MusicCompLow->Play();
	MusicCompHigh->Play();

	EnemyStackQuery();

	SetActorLocation(FVector::ZeroVector);

	Super::BeginPlay();
}

void ABaseLevelController::Tick(float DeltaSeconds)
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
		MusicCompExplore->SetVolumeMultiplier(((MusicParameter > 0.5f ? 0.0f : 1.0f - MusicParameter * 2.0f) + 0.001f) * MusicVolumeMultiplier);
		MusicCompLow->SetVolumeMultiplier(((MusicParameter < 0.5f ? MusicParameter * 2.0f : 1.0f) + 0.001f) * MusicVolumeMultiplier);
		MusicCompHigh->SetVolumeMultiplier(((MusicParameter < 0.5f ? 0.0f : (MusicParameter - 0.5f) * 2.0f) + 0.001f) * MusicVolumeMultiplier);
	}
	if (ChainedEnemies.Num() > 0)
	{
		for (FChainBoostVFXData& Data : ChainVFXData)
		{
			Data.DefaultVFX->SetNiagaraVariableVec3("start_pos", Data.Enemy1->GetActorLocation());
			Data.DefaultVFX->SetNiagaraVariableVec3("end_pos", Data.Enemy2->GetActorLocation());

			Data.DamageVFX->SetNiagaraVariableVec3("start_pos", Data.Enemy1->GetActorLocation());
			Data.DamageVFX->SetNiagaraVariableVec3("end_pos", Data.Enemy2->GetActorLocation());
		}
	}
	Super::Tick(DeltaSeconds);
}

int ABaseLevelController::GetCurMapIndex() const
{
	const FString CurMapName = GetWorld()->GetMapName();
	const FString Prefix = "UEDPIE_0_";
	for (int i = 0; i < LevelNames.Num(); ++i)
	{
		if (CurMapName == Prefix + LevelNames[i] || CurMapName == LevelNames[i])
		{
			return i;
		}
	}
	return -1;
}

void ABaseLevelController::ReadScoreboardData()
{
	if (Scores.Num())
	{
		return;
	}
	for (int i = 0; i < LevelData.Num(); ++i)
	{
		if (LevelData[i].Score > 0.0f)
		{
			FScoreboardData Data = { LevelData[i].Score, LevelData[i].DifficultyParameter, LevelData[i].LevelIndex };
			Scores.Add(Data);
		}
	}
	Scores.Sort();
}

void ABaseLevelController::LoadLevelData()
{
	UBaseRunDataSave* LoadedData = Cast<UBaseRunDataSave>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
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

void ABaseLevelController::SpawnEnemies()
{
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseEnemySpawnPoint::StaticClass(), SpawnPoints);
	for (int i = 0; i < SpawnPoints.Num(); ++i)
	{
		SpawnPoints.Swap(i, FMath::RandRange(i, SpawnPoints.Num() - 1));
	}
	float EnemyPercentage = GetOutputParameterFrom(DifficultyParameter, DifficultyParameterBounds, EnemyCountPercentageValues);
	BeginEnemyCount = FMath::CeilToInt(float(SpawnPoints.Num()) * EnemyPercentage);
	BeginEnemyCount = BeginEnemyCount > SpawnPoints.Num() ? SpawnPoints.Num() : BeginEnemyCount;
	CurLevelData.TotalEnemies = BeginEnemyCount;
	FewEnemiesEventCount = FMath::CeilToInt(FewEnemiesEventPercentage * (float)BeginEnemyCount);
	UE_LOG(LogTemp, Warning, TEXT("Enemies spawned: %d"), BeginEnemyCount);
	UE_LOG(LogTemp, Warning, TEXT("Few enemies event: %d"), FewEnemiesEventCount);
	for (int i = 0; i < BeginEnemyCount; ++i)
	{
		ABaseEnemySpawnPoint* SpawnPoint = Cast<ABaseEnemySpawnPoint>(SpawnPoints[i]);
		if (SpawnPoint)
		{
			SpawnEnemy(SpawnPoint);
		}
	}
}

void ABaseLevelController::SpawnEnemy(class ABaseEnemySpawnPoint* SpawnPoint, int Level, EEnemyLevelingType LevelingType)
{
	ABaseNPCSimpleChase* Enemy = SpawnPoint->SpawnEnemy(Level, LevelingType);
	if (Enemy)
	{
		Enemy->LevelController = this;
		SetEnemyParams(Enemy);
		Enemies.Add(Enemy);
	}
}

void ABaseLevelController::AddEnemy(class ABaseNPCSimpleChase* Enemy)
{
	Enemies.Add(Enemy);
}

void ABaseLevelController::RemoveEnemy(class ABaseNPCSimpleChase* Enemy)
{
	if (Enemies.Contains(Enemy))
	{
		Enemies.Remove(Enemy);
		++EnemiesKilled;
	}
	if (Enemies.Num() <= FewEnemiesEventCount)
	{
		FewEnemiesRemainingDelegate.Broadcast();
		UE_LOG(LogTemp, Warning, TEXT("Few enemies remaining!"));
	}
	if (!Enemies.Num())
	{
		OnAllEnemiesDead();
	}
}

void ABaseLevelController::UpdateMaxMultiplicator(float NewMultiplicator)
{
	if (NewMultiplicator > CurLevelData.MaxMultiplicator)
	{
		CurLevelData.MaxMultiplicator = NewMultiplicator;
	}
}

void ABaseLevelController::SetPlayerCharacter(class AHypercubeCharacter* PlayerCharacter)
{
	Player = PlayerCharacter;
	SetPlayerParams();
}

void ABaseLevelController::OnPlayerDeath()
{
	CurLevelData.bIsPlayerWon = false;
	TargetMusicParameter = 0.0f;
	SaveLevelData();
	GetWorld()->GetTimerManager().SetTimer(AfterLevelTimerHandle, this, &ABaseLevelController::AfterPlayerDeath, AfterPlayerDeathTime, false);
}

void ABaseLevelController::AfterPlayerDeath()
{
	UGameplayStatics::OpenLevel(GetWorld(), FName(*LevelNames[CurLevelIndex < 0 ? 0 : CurLevelIndex]));
}

void ABaseLevelController::OnAllEnemiesDead()
{
	CurLevelData.bIsPlayerWon = true;
	SaveLevelData();
	AllEnemiesDeadDelegate.Broadcast();
}

void ABaseLevelController::SaveLevelData()
{
	CurLevelData.Score = Player->Score;
	CurLevelData.EnemiesPercentageKilled = float(EnemiesKilled) / float(BeginEnemyCount);
	UpdateMaxMultiplicator(Player->DamageMultiplier);
	CurLevelData.OnDeathMultiplicator = Player->DamageMultiplier;
	CurLevelData.OnDeathEnemyChasing = Player->GetEnemyChasingCount();
	CurLevelData.PlayTime = UGameplayStatics::GetRealTimeSeconds(GetWorld());
	CurLevelData.DifficultyParameter = DifficultyParameter;
	CurLevelData.LevelIndex = CurLevelIndex;
	LevelData.Add(CurLevelData);
	UBaseRunDataSave* SaveGameInstance = Cast<UBaseRunDataSave>(UGameplayStatics::CreateSaveGameObject(UBaseRunDataSave::StaticClass()));
	if (SaveGameInstance)
	{
		SaveGameInstance->LevelDataArr = LevelData;
		UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveSlotName, 0);
	}
}

void ABaseLevelController::ClearLevelData()
{
	LevelData.Empty();
}

void ABaseLevelController::ReloadCurrentLevel()
{
	UGameplayStatics::OpenLevel(GetWorld(), FName(*LevelNames[CurLevelIndex < 0 ? 0 : CurLevelIndex]));
}

void ABaseLevelController::LoadNextLevel()
{
	UGameplayStatics::OpenLevel(GetWorld(), FName(*LevelNames[(CurLevelIndex + 1) % LevelNames.Num()]));
}

void ABaseLevelController::SetNoticeSoundTurnOff()
{
	bEnemyCanNoticeSound = false;
	GetWorld()->GetTimerManager().SetTimer(NoticeSoundTurnOffTimerHandle, this, &ABaseLevelController::OnEndNoticeSoundTurnedOff, NoticeSoundTurnOffTime, false);
}

void ABaseLevelController::OnEndNoticeSoundTurnedOff()
{
	bEnemyCanNoticeSound = true;
}

void ABaseLevelController::SetFootstepSoundTurnOff()
{
	bEnemyCanFootstepSound = false;
	GetWorld()->GetTimerManager().SetTimer(FootstepSoundTurnOffTimerHandle, this, &ABaseLevelController::OnEndFootstepSoundTurnedOff, FootstepSoundTurnOffTime, false);
}

void ABaseLevelController::OnEndFootstepSoundTurnedOff()
{
	bEnemyCanFootstepSound = true;
}

void ABaseLevelController::SetDeathSoundTurnOff()
{
	bEnemyCanDeathSound = false;
	GetWorld()->GetTimerManager().SetTimer(DeathSoundTurnOffTimerHandle, this, &ABaseLevelController::OnEndDeathSoundTurnedOff, DeathSoundTurnOffTime, false);
}

void ABaseLevelController::OnEndDeathSoundTurnedOff()
{
	bEnemyCanDeathSound = true;
}

float ABaseLevelController::GetDifficultyParameter()
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
	while (i >= 0 && !LevelData[i].bIsPlayerWon)
	{
		--i;
	}
	int DeathCount = LevelData.Num() - 1 - i;
	int OnDeathChasing = LevelData.Last().OnDeathEnemyChasing;
	float PlayTime = LevelData.Last().PlayTime;
	bool IsWon = LevelData.Last().bIsPlayerWon;

	float DeathCountParameter = GetDifficultyParameterFrom(DeathCount, DeathCountBounds, DeathCountValues) * DeathCountCost;
	float OnDeathChasingParameter = (IsWon ? 1.0f : GetDifficultyParameterFrom(OnDeathChasing, OnDeathEnemyAggroBounds, OnDeathEnemyAggroValues)) * OnDeathEnemyAggroCost;
	float PlayTimeParameter = (IsWon ? 1.0f : GetDifficultyParameterFrom(PlayTime, PlayTimeBounds, PlayTimeValues)) * PlayTimeCost;

	UE_LOG(LogTemp, Warning, TEXT("In: %d, %d, %f"), DeathCount, OnDeathChasing, PlayTime);
	UE_LOG(LogTemp, Warning, TEXT("Out: %f, %f, %f"), DeathCountParameter, OnDeathChasingParameter, PlayTimeParameter);
	return DeathCountParameter + OnDeathChasingParameter + PlayTimeParameter;
}

void ABaseLevelController::SetPlayerParams()
{
	Player->GetCharacterMovement()->MaxWalkSpeed *= GetOutputParameterFrom(DifficultyParameter, DifficultyParameterBounds, PlayerVelocityValues);
	Player->DamageMultiplierEnemyCost *= GetOutputParameterFrom(DifficultyParameter, DifficultyParameterBounds, PlayerDamageMultiplerValues);
	Player->Vampirism = GetOutputParameterFrom(DifficultyParameter, DifficultyParameterBounds, PlayerVampirismValues);
}

void ABaseLevelController::SetEnemyParams(class ABaseNPCSimpleChase* Enemy)
{
	Enemy->GetCharacterMovement()->MaxWalkSpeed *= GetOutputParameterFrom(DifficultyParameter, DifficultyParameterBounds, EnemyVelocityValues);
	Enemy->SimpleAttack.Damage *= GetOutputParameterFrom(DifficultyParameter, DifficultyParameterBounds, EnemyDamageValues);
	Enemy->GetNoticeCollision()->SetSphereRadius(Enemy->AggroRadius * GetOutputParameterFrom(DifficultyParameter, DifficultyParameterBounds, EnemyNoticeRadiusValues));
}

float ABaseLevelController::GetTargetMusicParameter() const
{
	if (Player->Health <= 0.0f)
	{
		return 0.0f;
	}
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

void ABaseLevelController::UpdateStackState()
{
	int EnemyCount = Player->GetEnemyChasingCount();

	EEnemyStackState PreviousStackState = StackState;

	if (EnemyCount < SoftStackEndEnemyCount)
	{
		StackState = EEnemyStackState::None;
	}
	else if ((StackState == EEnemyStackState::None && EnemyCount > SoftStackBeginEnemyCount && EnemyCount < HardStackBeginEnemyCount) || 
			 (StackState == EEnemyStackState::HardStack && EnemyCount < HardStackEndEnemyCount))
	{
		StackState = EEnemyStackState::SoftStack;
	}
	else if (EnemyCount > HardStackBeginEnemyCount)
	{
		StackState = EEnemyStackState::HardStack;
	}

	if (PreviousStackState == EEnemyStackState::HardStack && StackState != EEnemyStackState::HardStack)
	{
		bIsHardStackActive = false;
		HardStackDeactivationDelegate.Broadcast();
	}
}

void ABaseLevelController::SoftStack()
{
	TArray<ABaseNPCSimpleChase*> EnemyChasing = Player->GetEnemyChasingArray();

	if (!EnemyChasing.Num())
	{
		return;
	}

	int Index1 = FMath::RandRange(0, EnemyChasing.Num() - 1);
	ABaseNPCSimpleChase* Enemy1 = EnemyChasing[Index1];
	EnemyChasing.RemoveAt(Index1);

	int Index2 = GetEnemyIndexWithMinDistance(EnemyChasing, Enemy1);

	Enemy1->StackWith(EnemyChasing[Index2]);
	EnemyChasing[Index2]->StackWith(Enemy1);
}

void ABaseLevelController::HardStack()
{
	if (bIsHardStackActive)
	{
		return;
	}

	bIsHardStackActive = true;

	TArray<ABaseNPCSimpleChase*> EnemyChasing = Player->GetEnemyChasingArray();

	while (EnemyChasing.Num() > 1)
	{
		ABaseNPCSimpleChase* Enemy1 = EnemyChasing.Pop();

		int Index2 = GetEnemyIndexWithMinDistance(EnemyChasing, Enemy1);

		ABaseNPCSimpleChase* Enemy2 = EnemyChasing[Index2];
		EnemyChasing.RemoveAt(Index2);

		Enemy1->StackWith(Enemy2);
		Enemy2->StackWith(Enemy1);
	}
}

void ABaseLevelController::EnemyStackQuery()
{
	if (StackState == EEnemyStackState::SoftStack)
	{
		SoftStack();
	}
	else if (StackState == EEnemyStackState::HardStack)
	{
		HardStack();
	}

	GetWorld()->GetTimerManager().SetTimer(EnemyStackQueryTimerHandle, this, &ABaseLevelController::EnemyStackQuery, EnemyStackQueryFrequency, false);
}

void ABaseLevelController::StackEnemies(ABaseNPCSimpleChase* Enemy1, ABaseNPCSimpleChase* Enemy2)
{
	if (Enemy1 == Enemy2)
	{
		UE_LOG(LogTemp, Warning, TEXT("Trying to stack enemy with itself!"));
		return;
	}

	ABaseNPCSimpleChase* LowEnemy = Enemy1->GetEnemyLevel() < Enemy2->GetEnemyLevel() ? Enemy1 : Enemy2;
	ABaseNPCSimpleChase* HighEnemy = LowEnemy == Enemy1 ? Enemy2 : Enemy1;

	HighEnemy->IncreaseLevel(LowEnemy->GetEnemyLevel() + 1);
	HighEnemy->Health = HighEnemy->MaxHealth;

	HighEnemy->PlayStackVFX();

	Player->RemoveEnemyChasing(LowEnemy);
	RemoveEnemy(LowEnemy);
	RemoveEnemyFromChain(LowEnemy);

	LowEnemy->Destroy();
}

void ABaseLevelController::AddEnemyToChain(ABaseNPCSimpleChase* Enemy)
{
	if (ChainedEnemies.Contains(Enemy))
	{
		return;
	}

	ChainedEnemies.Add(Enemy);

	TArray<ABaseNPCSimpleChase*> ChainedEnemiesArray = ChainedEnemies.Array();

	ABaseNPCSimpleChase* Enemy2 = ChainedEnemiesArray.Num() == 1 ? Enemy : ChainedEnemiesArray[GetEnemyIndexWithMinDistance(ChainedEnemiesArray, Enemy)];

	FChainBoostVFXData NewData;

	NewData.Enemy1 = Enemy;
	NewData.Enemy2 = Enemy2;

	//FString ComponentName;
	//ComponentName.AppendInt(reinterpret_cast<unsigned long long>(Enemy));
	//ComponentName.AppendInt(reinterpret_cast<unsigned long long>(Enemy2));

	FTransform NewTransform(FRotator::ZeroRotator, FVector::ZeroVector);

	NewData.DefaultVFX = Cast<UNiagaraComponent>(AddComponentByClass(UNiagaraComponent::StaticClass(), false, NewTransform, false));
	NewData.DefaultVFX->SetAsset(ChainBoostNiagaraAsset->GetAsset());

	NewData.DamageVFX = Cast<UNiagaraComponent>(AddComponentByClass(UNiagaraComponent::StaticClass(), false, NewTransform, false));
	NewData.DamageVFX->SetAsset(ChainBoostDamageNiagaraAsset->GetAsset());

	ChainVFXData.Add(NewData);
}

void ABaseLevelController::RemoveEnemyFromChain(ABaseNPCSimpleChase* Enemy)
{
	if (!ChainedEnemies.Contains(Enemy))
	{
		return;
	}

	ChainedEnemies.Remove(Enemy);

	int i = 0;
	while (i < ChainVFXData.Num())
	{
		if (ChainVFXData[i].Enemy1 == Enemy || ChainVFXData[i].Enemy2 == Enemy)
		{
			ChainVFXData[i].DefaultVFX->DestroyComponent();
			ChainVFXData[i].DamageVFX->DestroyComponent();
			ChainVFXData.RemoveAt(i);
		}
		else
		{
			++i;
		}
	}
}

void ABaseLevelController::DealChainDamageExcept(class ABaseNPCSimpleChase* Enemy, float Damage)
{
	for (ABaseNPCSimpleChase* ChainedEnemy : ChainedEnemies)
	{
		if (ChainedEnemy != Enemy)
		{
			ChainedEnemy->TakeDamage(Damage * ChainDamageMultiplier, true);
		}
	}
}

int GetEnemyIndexWithMinDistance(const TArray<ABaseNPCSimpleChase*>& Enemies, ABaseNPCSimpleChase* EnemyToCompare)
{
	FVector Location = EnemyToCompare->GetActorLocation();

	float MinDistance = FVector::Distance(Enemies[0]->GetActorLocation(), Location);
	int Index = 0;

	for (int i = 1; i < Enemies.Num(); ++i)
	{
		if (Enemies[i] == EnemyToCompare)
		{
			continue;
		}

		float Distance = FVector::Distance(Enemies[i]->GetActorLocation(), Location);
		if (Distance < MinDistance)
		{
			MinDistance = Distance;
			Index = i;
		}
	}

	return Index;
}

FString ABaseLevelController::GetScoreboard(int Num)
{
	ReadScoreboardData();
	if (!Scores.Num())
	{
		return FString("");
	}
	FString Result = "Scoreboard:\n\n      Level        Difficulty    Score\n";
	Num = Num > Scores.Num() ? Scores.Num() : Num;
	for (int i = 0; i < Num; ++i)
	{
		FScoreboardData Data = Scores[Scores.Num() - 1 - i];
		Result.AppendInt(i + 1);
		Result += (i + 1 < 10) ? FString(".  ") : FString(". ");
		FString LevelName = (Data.LevelIndex >= 0 && Data.LevelIndex < LevelNamesToShow.Num()) ? LevelNamesToShow[Data.LevelIndex] : FString("????");
		Result += LevelName;
		for (int j = 0; j < 12 - LevelName.Len(); ++j)
		{
			Result.AppendChar(' ');
		}
		FString DifficultyString = FloatToFString(Data.DifficultyParameter);
		Result += DifficultyString;
		int SpaceCount = DifficultyString.Len() < 3 ? 20 : 13;
		for (int j = 0; j < SpaceCount; ++j)
		{
			Result.AppendChar(' ');
		}
		Result.AppendInt(FMath::RoundToInt(Data.Score));
		Result.AppendChar('\n');
	}
	return Result;
}

FString ABaseLevelController::GetScoreboardEnumerate(int Num)
{
	ReadScoreboardData();
	int Count = Scores.Num() > Num ? Num : Scores.Num();
	FString Result("");
	for (int i = 0; i < Count; ++i)
	{
		Result.AppendInt(i + 1);
		Result.AppendChar('\n');
	}
	return Result;
}

FString ABaseLevelController::GetScoreboardLevels(int Num)
{
	ReadScoreboardData();
	int Count = Scores.Num() > Num ? Num : Scores.Num();
	FString Result("Level\n\n");
	for (int i = 0; i < Count; ++i)
	{
		Result += (Scores[i].LevelIndex >= 0 && Scores[i].LevelIndex < LevelNamesToShow.Num()) ? LevelNamesToShow[Scores[i].LevelIndex] : FString("????");
		Result.AppendChar('\n');
	}
	return Result;
}

FString ABaseLevelController::GetScoreboardScores(int Num)
{
	ReadScoreboardData();
	int Count = Scores.Num() > Num ? Num : Scores.Num();
	FString Result("Score\n\n");
	for (int i = 0; i < Count; ++i)
	{
		Result.AppendInt(FMath::RoundToInt(Scores[i].Score));
		Result.AppendChar('\n');
	}
	return Result;
}

FString ABaseLevelController::GetScoreboardDiffs(int Num)
{
	ReadScoreboardData();
	int Count = Scores.Num() > Num ? Num : Scores.Num();
	FString Result("Difficulty\n\n");
	for (int i = 0; i < Count; ++i)
	{
		Result += FloatToFString(Scores[i].DifficultyParameter);
		Result.AppendChar('\n');
	}
	return Result;
}

FString ABaseLevelController::GetDifficultyBrief() const
{
	FString Result = "Difficulty parameter: " + FloatToFString(DifficultyParameter);
	Result += FString("\n\n\nPlayer stats:\n\nSpeed: x") + FloatToFString(GetOutputParameterFrom(DifficultyParameter, DifficultyParameterBounds, PlayerVelocityValues));
	Result += FString("\nDamage stats growing speed: x") + FloatToFString(GetOutputParameterFrom(DifficultyParameter, DifficultyParameterBounds, PlayerDamageMultiplerValues));
	Result += FString("\nVampirism: ");
	Result.AppendInt(int(GetOutputParameterFrom(DifficultyParameter, DifficultyParameterBounds, PlayerVampirismValues) * 100.0f));
	Result += FString("%\n\n\nEnemy stats:\n\nSpeed: x") + FloatToFString(GetOutputParameterFrom(DifficultyParameter, DifficultyParameterBounds, EnemyVelocityValues));
	Result += FString("\nDamage: x") + FloatToFString(GetOutputParameterFrom(DifficultyParameter, DifficultyParameterBounds, EnemyDamageValues));
	Result += FString("\nNotice radius: x") + FloatToFString(GetOutputParameterFrom(DifficultyParameter, DifficultyParameterBounds, EnemyNoticeRadiusValues));
	Result += FString("\nEnemy Count: ");
	Result.AppendInt(int(GetOutputParameterFrom(DifficultyParameter, DifficultyParameterBounds, EnemyCountPercentageValues) * 100.0f));
	Result += FString("%\nEnemy Leveling: ");
	Result.AppendInt(int(EnemyLevelingPercentage * 100.0f));
	Result.AppendChar('%');
	return Result;
}

FString FloatToFString(float Val)
{
	FString Result("");
	int WholePart = FMath::FloorToInt(Val);
	int AfterPoint = FMath::FloorToInt((Val - FMath::FloorToFloat(Val)) * 100.0f);
	Result.AppendInt(WholePart);
	if (!AfterPoint)
	{
		return Result;
	}
	Result.AppendChar('.');
	Result.AppendInt(AfterPoint);
	return Result;
}