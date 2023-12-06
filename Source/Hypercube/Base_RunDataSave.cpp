#include "Base_RunDataSave.h"

void FLevelData::Log()
{
	UE_LOG(LogTemp, Warning, TEXT("PlayerWon: %d, Score: %f, Killed: %f, MaxMult: %f, MultOnDeath: %f"), int(PlayerWon), Score, EnemiesPercentageKilled, MaxMultiplicator, OnDeathMultiplicator);
}