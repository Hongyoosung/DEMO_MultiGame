#include "MultiGameMode.h"
#include "DEMO_MultiGamePlayerController.h"
#include "DEMO_MultiGameCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "HAL/ThreadManager.h"
#include "Tasks/TAttackTask.h"


AMultiGameMode::AMultiGameMode()
{
	// 스레드풀 생성
	ThreadPool = FQueuedThreadPool::Allocate();
	ThreadPool->Create(4, 32 * 1024, TPri_Normal, TEXT("AttackThreadPool"));
}


void AMultiGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Initialize AttackTaskPool
	InitializeAttackTaskPool();

	// Create AntiCheatManager
	AntiCheatManager = UAntiCheatManager::CreateManager();
}

void AMultiGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 남아있는 태스크 정리
	FTAttackTask* Task;
	while (AttackTaskPool.Dequeue(Task))
	{
		delete Task;
	}

	// 스레드풀 정리
	if (ThreadPool)
	{
		ThreadPool->Destroy();
		delete ThreadPool;
		ThreadPool = nullptr;
	}
	
	Super::EndPlay(EndPlayReason);
}

void AMultiGameMode::InitializeAttackTaskPool()
{
	// 태스크풀에 최대 태스크 추가
	for (int32 i = 0; i < MAX_ATTACK_TASKS; ++i)
	{
		AttackTaskPool.Enqueue(new FTAttackTask());
	}
}

void AMultiGameMode::AdjustThreadPoolSize()
{
	
}

void AMultiGameMode::ExecuteAttackTask(APlayerCharacter* Player)
{
	if (ThreadPool)
	{
		// 태스크풀에서 태스크 가져오기 또는 생성
		FTAttackTask* Task = GetOrCreateAttackTask();
		Task->Initialize(Player);

		// 태스크 완료 콜백 설정
		Task->SetCompletionCallback([this](FTAttackTask* CompletedTask)
		{
			ReturnAttackTaskToPool(CompletedTask);
		});

		// 스레드풀에 태스크 추가
		ThreadPool->AddQueuedWork(Task);
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