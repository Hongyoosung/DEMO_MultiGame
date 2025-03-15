// MultiGameMode.h
#pragma once

#include "CoreMinimal.h"
#include "DEMO_MultiGameGameMode.h"
#include "GameFramework/GameModeBase.h"
#include "MultiGameMode.generated.h"


class FCustomQueuedThreadPool;
class UAntiCheatManager;
class FTAttackTask;
class FTAcquireItemTask;
class FTUseItemTask;


UCLASS(minimalapi)
class AMultiGameMode : public ADEMO_MultiGameGameMode
{
	GENERATED_BODY()

public:
	AMultiGameMode();

	virtual void		BeginPlay() override;
	virtual void		EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void		GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	
	// Importing tasks from a task pool
	FTAttackTask*		GetOrCreateAttackTask();
	FTAcquireItemTask*	GetOrCreateAcquireItemTask();
	FTUseItemTask*		GetOrCreateUseItemTask();


	// Assigning tasks to a threadpool for processing
	void				ExecuteAttackTask			(FTAttackTask* Task);
	void				ExecuteAcquireItemTask		(FTAcquireItemTask* Task);
	void				ExecuteUseItemTask			(FTUseItemTask* Task);

	
	// Task pool and antcheat manager getters
	FORCEINLINE FCustomQueuedThreadPool*	GetThreadPool		()	const {		return ThreadPool;		  }
	FORCEINLINE UAntiCheatManager*			GetAntiCheatManager	()	const {		return AntiCheatManager;  }
	

private:
	void InitializeTaskPools	();
	void AdjustThreadPoolSize	();
	void ReturnTaskToPool		(FTAttackTask* Task);
	void ReturnTaskToPool		(FTAcquireItemTask* Task);
	void ReturnTaskToPool		(FTUseItemTask* Task);

	
private:
	UPROPERTY(Replicated)
	UAntiCheatManager* AntiCheatManager;

	FCustomQueuedThreadPool* ThreadPool;
	
	// Task pool lock
	FCriticalSection TaskPoolLock;

	FTimerHandle ThreadPoolAdjustmentTimer;

	// Task pools
	TQueue<FTAttackTask*> AttackTaskPool;
	TQueue<FTAcquireItemTask*> AcquireItemTaskPool;
	TQueue<FTUseItemTask*> UseItemTaskPool;
	
	static constexpr int32 MAX_TASKS = 10;
};