
#include "Base_RunDataSave.h"

void FLevelData::Log()
{
	UE_LOG(LogTemp, Warning, TEXT("Score: %f, Killed: %d, MaxMult: %f, MultOnDeath: %f"), Score, EnemiesKilled, MaxMultiplicator, OnDeathMultiplicator);
}