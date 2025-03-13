#pragma once

#include "CoreMinimal.h"
#include "DEMO_MultiGameGameMode.h"
#include "GameFramework/GameModeBase.h"
#include "MultiGameMode.generated.h"


class FTAcquireItemTask;
class FTAttackTask;
class FTUseItemTask;
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
	FTAttackTask*				GetOrCreateAttackTask();
	FTAcquireItemTask*			GetOrCreateAcquireItemTask();
	FTUseItemTask*				GetOrCreateUseItemTask();
	
	void ExecuteAttackTask			(FTAttackTask* Task);
	void ExecuteAcquireItemTask		(FTAcquireItemTask* Task);
	void ExecuteUseItemTask			(FTUseItemTask* Task);


private:
	void InitializeAttackTaskPool();
	
	void AdjustThreadPoolSize() const;

	void ReturnTaskToPool(FTAttackTask* Task);
	void ReturnTaskToPool(FTAcquireItemTask* Task);
	void ReturnTaskToPool(FTUseItemTask* Task);
	
	

private:
	UPROPERTY()
	UAntiCheatManager*			AntiCheatManager;
	
	FCustomQueuedThreadPool*	ThreadPool;
	
	TQueue<FTAttackTask*>		AttackTaskPool;
	TQueue<FTAcquireItemTask*>	AcquireItemTaskPool;
	TQueue<FTUseItemTask*>		UseItemTaskPool;
	
	FCriticalSection			TaskPoolLock;

	FTimerHandle				ThreadPoolAdjustmentTimer;

	static constexpr int32		MAX_TASKS = 10;
};


