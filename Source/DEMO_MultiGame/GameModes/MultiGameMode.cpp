#include "C:\Users\Foryoucom\Documents\Unreal Projects\DEMO_MultiGame\Intermediate\Build\Win64\x64\DEMO_MultiGameEditor\Development\UnrealEd\SharedPCH.UnrealEd.Project.ValApi.Cpp20.h"
#include "MultiGameMode.h"
#include "DEMO_MultiGamePlayerController.h"
#include "DEMO_MultiGameCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "HAL/ThreadManager.h"
#include "Tasks/TAttackTask.h"


AMultiGameMode::AMultiGameMode()
{
	ThreadPool = FQueuedThreadPool::Allocate();
	ThreadPool->Create(4, 32 * 1024, TPri_Normal, TEXT("AttackThreadPool"));
}


void AMultiGameMode::BeginPlay()
{
	Super::BeginPlay();

	InitializeAttackTaskPool();
}

void AMultiGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 남아있는 태스크 정리
	FTAttackTask* Task;
	while (AttackTaskPool.Dequeue(Task))
	{
		delete Task;
	}

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
	for (int32 i = 0; i < MAX_ATTACK_TASKS; ++i)
	{
		AttackTaskPool.Enqueue(new FTAttackTask());
	}
}

void AMultiGameMode::ExecuteAttackTask(APlayerCharacter* Player)
{
	if (ThreadPool)
	{
		FTAttackTask* Task = GetOrCreateAttackTask();
		Task->Initialize(Player);
		
		Task->SetCompletionCallback([this](FTAttackTask* CompletedTask)
		{
			ReturnAttackTaskToPool(CompletedTask);
		});
		
		ThreadPool->AddQueuedWork(Task);
	}
}

FTAttackTask* AMultiGameMode::GetOrCreateAttackTask()
{
	FTAttackTask* Task = nullptr;
	{
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
	FScopeLock Lock(&AttackTaskPoolLock);
	AttackTaskPool.Enqueue(Task);
}