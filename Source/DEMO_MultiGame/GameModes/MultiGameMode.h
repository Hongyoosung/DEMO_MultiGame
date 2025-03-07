#pragma once

#include "CoreMinimal.h"
#include "DEMO_MultiGameGameMode.h"
#include "GameFramework/GameModeBase.h"
#include "MultiGameMode.generated.h"


class FTAttackTask;
class APlayerCharacter;
class UAntiCheatManager;


UCLASS(minimalapi)
class AMultiGameMode : public ADEMO_MultiGameGameMode
{
	GENERATED_BODY()
	
public:
	AMultiGameMode();

	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	
	FQueuedThreadPool*	GetThreadPool()			const	{		return ThreadPool;			}
	UAntiCheatManager*	GetAntiCheatManager()	const	{		return AntiCheatManager;	}

	
	void ExecuteAttackTask(APlayerCharacter* Player);

private:
	void InitializeAttackTaskPool();
	
	void AdjustThreadPoolSize();

	void ReturnAttackTaskToPool(FTAttackTask* Task);
	
	FTAttackTask* GetOrCreateAttackTask();

private:
	UPROPERTY()
	UAntiCheatManager* AntiCheatManager;
	
	FQueuedThreadPool* ThreadPool;
	
	TQueue<FTAttackTask*> AttackTaskPool;
	
	FCriticalSection AttackTaskPoolLock;

	static constexpr int32 MAX_ATTACK_TASKS = 10;
};
