#include "MultiGameMode.h"
#include "DEMO_MultiGamePlayerController.h"
#include "DEMO_MultiGameCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "HAL/ThreadManager.h"
#include "Tasks/TAttackTask.h"
#include "ThreadPool/CustomQueuedThreadPool.h"


AMultiGameMode::AMultiGameMode()
{
	// 스레드풀 생성
	ThreadPool = FCustomQueuedThreadPool::Allocate();
	ThreadPool->Create(4, 32 * 1024, TPri_Normal, TEXT("AttackThreadPool"));
}


void AMultiGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Initialize AttackTaskPool
	InitializeAttackTaskPool();

	// Create AntiCheatManager
	AntiCheatManager = UAntiCheatManager::CreateManager();
	
	// 2초마다 AdjustThreadPoolSize 호출
	//GetWorldTimerManager().SetTimer(ThreadPoolAdjustmentTimer, this, &AMultiGameMode::AdjustThreadPoolSize, 2.0f, true);
}

void AMultiGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(ThreadPoolAdjustmentTimer);
	
	// Clean up thread pool
	if (ThreadPool)
	{
		ThreadPool->WaitForCompletion();
		ThreadPool->Destroy();
		delete ThreadPool;
		ThreadPool = nullptr;
	}

	// Clean up remaining tasks
	FTAttackTask* Task;
	while (AttackTaskPool.Dequeue(Task))
	{
		delete Task;
	}
	
	Super::EndPlay(EndPlayReason);
}

void AMultiGameMode::InitializeAttackTaskPool()
{
	// Add attack tasks to the pool
	for (int32 i = 0; i < MAX_ATTACK_TASKS; ++i)
	{
		AttackTaskPool.Enqueue(new FTAttackTask());
	}
}

void AMultiGameMode::AdjustThreadPoolSize() const
{
	const int32 ActiveTasks = ThreadPool->GetActiveTaskCount();
	const int32 CurrentThreads = ThreadPool->GetNumThreads();

	if (ActiveTasks > CurrentThreads * 0.8f && CurrentThreads < 8)
	{
		// 개별 스레드 추가
		ThreadPool->AddThread(32 * 1024, TPri_Normal, TEXT("AttackThreadPool"));
	}
	else if (ActiveTasks < CurrentThreads * 0.2f && CurrentThreads > 2)
	{
		// 개별 스레드 제거
		ThreadPool->RemoveThread();
	}
}

void AMultiGameMode::ExecuteAttackTask(APlayerCharacter* Player)
{
	if (ThreadPool)
	{
		// Import or create tasks from a taskpool
		FTAttackTask* Task = GetOrCreateAttackTask();
		Task->InitializePlayerValues(Player);

		// Setting up task completion callbacks
		Task->SetCompletionCallback([this](FTAttackTask* CompletedTask)
		{
			CompletedTask->Init();
			ReturnAttackTaskToPool(CompletedTask);
		});

		// Adding a Task to a Threadpool
		ThreadPool->AddQueuedWork(Task, EQueuedWorkPriority::Normal);
	}
}

FTAttackTask* AMultiGameMode::GetOrCreateAttackTask()
{
	FTAttackTask* Task = nullptr;
	{
		// 태스크풀에서 태스크 가져오기, 없으면 생성, 태스크풀에 RAII 락 수행
		FScopeLock Lock(&AttackTaskPoolLock);
		if (!AttackTaskPool.Dequeue(Task))
		{
			Task = new FTAttackTask();
		}
	}
	
	return Task;
}

void AMultiGameMode::ReturnAttackTaskToPool(FTAttackTask* Task)
{
	// 태스크풀에 RAII 락 수행
	FScopeLock Lock(&AttackTaskPoolLock);

	// 태스크풀로 반환
	AttackTaskPool.Enqueue(Task);
}