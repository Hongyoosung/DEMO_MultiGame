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

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 태스크 풀 및 안티치트 매니저 접근자
	FCustomQueuedThreadPool* GetThreadPool() const { return ThreadPool; }
	UAntiCheatManager* GetAntiCheatManager() const { return AntiCheatManager; }

	FTAttackTask* GetOrCreateAttackTask();
	FTAcquireItemTask* GetOrCreateAcquireItemTask();
	FTUseItemTask* GetOrCreateUseItemTask();

	void ExecuteAttackTask(FTAttackTask* Task);
	void ExecuteAcquireItemTask(FTAcquireItemTask* Task);
	void ExecuteUseItemTask(FTUseItemTask* Task);

private:
	void InitializeTaskPools();
	void AdjustThreadPoolSize();
	void ReturnTaskToPool(FTAttackTask* Task);
	void ReturnTaskToPool(FTAcquireItemTask* Task);
	void ReturnTaskToPool(FTUseItemTask* Task);

private:
	UPROPERTY(Replicated)
	UAntiCheatManager* AntiCheatManager;

	FCustomQueuedThreadPool* ThreadPool;

	TQueue<FTAttackTask*> AttackTaskPool;
	TQueue<FTAcquireItemTask*> AcquireItemTaskPool;
	TQueue<FTUseItemTask*> UseItemTaskPool;

	FCriticalSection TaskPoolLock;

	FTimerHandle ThreadPoolAdjustmentTimer;

	static constexpr int32 MAX_TASKS = 10;
};