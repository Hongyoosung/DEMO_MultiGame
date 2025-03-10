﻿#include "MultiGameMode.h"
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
	AntiCheatManager = GetGameInstance()->GetSubsystem<UAntiCheatManager>();
	
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
	if (!ThreadPool || !IsValid(Player))
	{
		TESTLOG(Error, TEXT("Invalid ThreadPool or Player"));
		return;
	}

	// 태스크 가져오기
	FTAttackTask* Task = GetOrCreateAttackTask();
	if (!Task)
	{
		TESTLOG(Error, TEXT("Failed to get or create attack task"));
		return;
	}
    
	// 태스크 초기화
	Task->InitializePlayerValues(Player);

	// 완료 콜백 설정
	Task->SetCompletionCallback([this, Task](FTAttackTask* CompletedTask)
	{
		check(CompletedTask == Task); // 태스크 일관성 확인
        
		if (!CompletedTask->IsReturnedToPool())
		{
			// 태스크가 아직 반환되지 않았을 경우에만 초기화 및 반환
			ReturnAttackTaskToPool(CompletedTask);
		}
		else
		{
			TESTLOG(Warning, TEXT("Task already returned to pool, skipping"));
		}
	});

	// 스레드풀에 태스크 추가
	ThreadPool->AddQueuedWork(Task, EQueuedWorkPriority::Normal);
}

FTAttackTask* AMultiGameMode::GetOrCreateAttackTask()
{
	FTAttackTask* Task = nullptr;

	{
		// 태스크풀에서 태스크 가져오기, 없으면 생성
		FScopeLock Lock(&AttackTaskPoolLock);
		if (!AttackTaskPool.Dequeue(Task))
		{
			// 풀에서 사용할 수 있는 태스크가 없으면 새로 생성
			Task = new FTAttackTask();
		}
	}

	if (Task)
	{
		// 풀에서 꺼낸 상태로 표시
		Task->SetReturnedToPool(false);
	}

	return Task;
}

void AMultiGameMode::ReturnAttackTaskToPool(FTAttackTask* Task)
{
	if (!Task)
	{
		TESTLOG(Error, TEXT("Attempting to return null task to pool"));
		return;
	}

	// 이미 풀에 반환된 태스크를 다시 반환하지 않도록 함
	if (Task->IsReturnedToPool())
	{
		TESTLOG(Warning, TEXT("Task already returned to pool"));
		return;
	}

	// 실행 중인 상태에서는 반환하지 않음
	if (Task->IsTaskRunning())
	{
		TESTLOG(Warning, TEXT("Cannot return a running task to the pool"));
		return;
	}

	// 안전하게 초기화 후 반환
	FScopeLock Lock(&AttackTaskPoolLock);
	Task->Init();
	Task->SetReturnedToPool(true);
	AttackTaskPool.Enqueue(Task);
}