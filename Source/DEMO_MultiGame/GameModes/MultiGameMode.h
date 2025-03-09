#pragma once

#include "CoreMinimal.h"
#include "DEMO_MultiGameGameMode.h"
#include "GameFramework/GameModeBase.h"
#include "MultiGameMode.generated.h"


class FTAttackTask;
class APlayerCharacter;
class UAntiCheatManager;
class FCustomQueuedThreadPool;


UCLASS(minimalapi)
class AMultiGameMode : public ADEMO_MultiGameGameMode
{
	GENERATED_BODY()
	
public:
	AMultiGameMode();

	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	
	FCustomQueuedThreadPool*	GetThreadPool()			const	{		return ThreadPool;			}
	UAntiCheatManager*			GetAntiCheatManager()	const	{		return AntiCheatManager;	}

	
	void ExecuteAttackTask(APlayerCharacter* Player);

private:
	void InitializeAttackTaskPool();
	
	void AdjustThreadPoolSize() const;

	void ReturnAttackTaskToPool(FTAttackTask* Task);
	
	FTAttackTask* GetOrCreateAttackTask();

private:
	UPROPERTY()
	UAntiCheatManager* AntiCheatManager;
	
	FCustomQueuedThreadPool* ThreadPool;
	
	TQueue<FTAttackTask*> AttackTaskPool;
	
	FCriticalSection AttackTaskPoolLock;

	FTimerHandle ThreadPoolAdjustmentTimer;

	static constexpr int32 MAX_ATTACK_TASKS = 10;
};
