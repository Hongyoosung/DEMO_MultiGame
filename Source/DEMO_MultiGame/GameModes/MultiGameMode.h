#pragma once

#include "CoreMinimal.h"
#include "DEMO_MultiGameGameMode.h"
#include "GameFramework/GameModeBase.h"
#include "MultiGameMode.generated.h"


class FTAttackTask;
class APlayerCharacter;


UCLASS(minimalapi)
class AMultiGameMode : public ADEMO_MultiGameGameMode
{
	GENERATED_BODY()
	
public:
	AMultiGameMode();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	FQueuedThreadPool* GetThreadPool() const { return ThreadPool; }
	void ExecuteAttackTask(APlayerCharacter* Player);

private:
	void InitializeAttackTaskPool();
	FTAttackTask* GetOrCreateAttackTask();
	void ReturnAttackTaskToPool(FTAttackTask* Task);
	
private:
	FQueuedThreadPool* ThreadPool;
	TQueue<FTAttackTask*> AttackTaskPool;
	FCriticalSection AttackTaskPoolLock;

	static const int32 MAX_ATTACK_TASKS = 10;
};
