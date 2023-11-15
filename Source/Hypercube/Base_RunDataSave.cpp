
#include "Base_RunDataSave.h"

void FLevelData::Log()
{
	UE_LOG(LogTemp, Warning, TEXT("PlayerWon: %d, Score: %f, Killed: %d, MaxMult: %f, MultOnDeath: %f"), int(PlayerWon), Score, EnemiesKilled, MaxMultiplicator, OnDeathMultiplicator);
}