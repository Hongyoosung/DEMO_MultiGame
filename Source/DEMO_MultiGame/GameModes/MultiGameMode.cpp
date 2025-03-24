// MultiGameState.cpp
#include "MultiGameMode.h"
#include "DEMO_MultiGame.h"
#include "Tasks/TAttackTask.h"
#include "Tasks/TAcquireItemTask.h"
#include "Tasks/TUseItemTask.h"
#include "ThreadPool/CustomQueuedThreadPool.h"
#include "Managers/AntiCheatManager.h"
#include "Net/UnrealNetwork.h"


AMultiGameMode::AMultiGameMode() : AntiCheatManager(nullptr), ThreadPool(nullptr)
{
    PrimaryActorTick.bCanEverTick = false;
}


void AMultiGameMode::BeginPlay()
{
    Super::BeginPlay();

#ifdef UE_SERVER

    ThreadPool = FCustomQueuedThreadPool::Allocate();
    ThreadPool->Create(4, 32 * 1024, TPri_Normal, TEXT("GameStateThreadPool"));

    
    // Create Thread Pool
    if (!ThreadPool)
    {
        ThreadPool = FCustomQueuedThreadPool::Allocate();
        ThreadPool->Create(4, 32 * 1024, TPri_Normal, TEXT("GameStateThreadPool"));
    }

    
    InitializeTaskPools();
    
    AntiCheatManager = GetGameInstance()->GetSubsystem<UAntiCheatManager>();
    if (!AntiCheatManager)
    {
        TESTLOG(Error, TEXT("Failed to get AntiCheatManager"));
    }

#endif
}


void AMultiGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    
#ifdef UE_SERVER
    
    if (ThreadPool)
    {
        ThreadPool->WaitForCompletion();
        ThreadPool->Destroy();
        delete ThreadPool;
        ThreadPool = nullptr;
    }
    else
    {
        TESTLOG(Error, TEXT("Failed to get GameState"));
    }


    // Clean up task pools
    FTAttackTask* AttackTask;
    while (AttackTaskPool.Dequeue(AttackTask)) { delete AttackTask;  AttackTask = nullptr; }
    FTAcquireItemTask* AcquireTask;
    while (AcquireItemTaskPool.Dequeue(AcquireTask)) { delete AcquireTask;  AcquireTask = nullptr; }
    FTUseItemTask* UseTask;
    while (UseItemTaskPool.Dequeue(UseTask)) { delete UseTask;  UseTask = nullptr; }
    
#endif

    Super::EndPlay(EndPlayReason);
}


void AMultiGameMode::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AMultiGameMode, AntiCheatManager);
}


void AMultiGameMode::InitializeTaskPools()
{
#ifdef UE_SERVER

    for (int32 i = 0; i < MAX_TASKS; ++i)
    {
        AttackTaskPool.Enqueue(new FTAttackTask());
        AcquireItemTaskPool.Enqueue(new FTAcquireItemTask());
        UseItemTaskPool.Enqueue(new FTUseItemTask());
    }
    
#endif
}


void AMultiGameMode::AdjustThreadPoolSize()
{
    
#ifdef UE_SERVER

    const int32 ActiveTasks = ThreadPool->GetActiveTaskCount();
    const int32 CurrentThreads = ThreadPool->GetNumThreads();

    if (ActiveTasks > CurrentThreads * 0.8f && CurrentThreads < 8)
    {
        ThreadPool->AddThread(32 * 1024, TPri_Normal, TEXT("GameStateThreadPool"));
    }
    else if (ActiveTasks < CurrentThreads * 0.2f && CurrentThreads > 2)
    {
        ThreadPool->RemoveThread();
    }
    
#endif
    
}


void AMultiGameMode::ExecuteAttackTask(FTAttackTask* Task)
{
    
#ifdef UE_SERVER
    
    if (!Task || !ThreadPool)
    {
        TESTLOG(Error, TEXT("Invalid Task or ThreadPool"));
        return;
    }
    
    Task->SetCompletionCallback([this, Task](FTAttackTask* CompletedTask)
    {
        if (!CompletedTask->IsReturnedToPool())
        {
            ReturnTaskToPool(CompletedTask);
        }
    });
    
    ThreadPool->AddQueuedWork(Task, EQueuedWorkPriority::Normal);
    
#endif
    
}


void AMultiGameMode::ExecuteAcquireItemTask(FTAcquireItemTask* Task)
{
    
#ifdef UE_SERVER

    if (!Task || !ThreadPool)
    {
        TESTLOG(Error, TEXT("Invalid Task or ThreadPool"));
        return;
    }
    
    Task->SetCompletionCallback([this, Task](FTAcquireItemTask* CompletedTask)
    {
        if (!CompletedTask->IsReturnedToPool())
        {
            ReturnTaskToPool(CompletedTask);
        }
    });
    ThreadPool->AddQueuedWork(Task, EQueuedWorkPriority::Normal);
    
#endif
    
}

void AMultiGameMode::ExecuteUseItemTask(FTUseItemTask* Task)
{
    
#ifdef UE_SERVER

    if (!Task || !ThreadPool)
    {
        TESTLOG(Error, TEXT("Invalid Task or ThreadPool"));
        return;
    }
    
    Task->SetCompletionCallback([this, Task](FTUseItemTask* CompletedTask)
    {
        if (!CompletedTask->IsReturnedToPool())
        {
            ReturnTaskToPool(CompletedTask);
        }
    });
    
    ThreadPool->AddQueuedWork(Task, EQueuedWorkPriority::Normal);
    
#endif
}


FTAttackTask* AMultiGameMode::GetOrCreateAttackTask()
{
    
#ifdef UE_SERVER
    
    FTAttackTask* Task = nullptr;
    
    {
        FScopeLock Lock(&TaskPoolLock);
        
        if (!AttackTaskPool.Dequeue(Task))
        {
            Task = new FTAttackTask();
        }
    }
    
    if (Task) Task->SetReturnedToPool(false);
    
    return Task;
    
#else
    return nullptr;
#endif
}


FTAcquireItemTask* AMultiGameMode::GetOrCreateAcquireItemTask()
{
    
#ifdef UE_SERVER
    
    FTAcquireItemTask* Task = nullptr;
    
    {
        FScopeLock Lock(&TaskPoolLock);
        
        if (!AcquireItemTaskPool.Dequeue(Task))
        {
            Task = new FTAcquireItemTask();
        }
    }
    
    if (Task) Task->SetReturnedToPool(false);
    
    return Task;

#else
    return nullptr;
#endif
}


FTUseItemTask* AMultiGameMode::GetOrCreateUseItemTask()
{
    
#ifdef UE_SERVER
    
    FTUseItemTask* Task = nullptr;
    
    {
        FScopeLock Lock(&TaskPoolLock);
        if (!UseItemTaskPool.Dequeue(Task))
        {
            Task = new FTUseItemTask();
        }
    }
    
    if (Task) Task->SetReturnedToPool(false);
    
    return Task;

#else
    return nullptr;
#endif
    
}


void AMultiGameMode::ReturnTaskToPool(FTAttackTask* Task)
{
    
#ifdef UE_SERVER
    
    if (!Task || Task->IsReturnedToPool() || Task->IsTaskRunning()) return;
    
    FScopeLock Lock(&TaskPoolLock);
    Task->Init();
    Task->SetReturnedToPool(true);
    AttackTaskPool.Enqueue(Task);

#endif
    
}

void AMultiGameMode::ReturnTaskToPool(FTAcquireItemTask* Task)
{
    
#ifdef UE_SERVER
    
    if (!Task || Task->IsReturnedToPool() || Task->IsTaskRunning()) return;
    
    FScopeLock Lock(&TaskPoolLock);
    Task->Init();
    Task->SetReturnedToPool(true);
    AcquireItemTaskPool.Enqueue(Task);

#endif
    
}


void AMultiGameMode::ReturnTaskToPool(FTUseItemTask* Task)
{
    
#ifdef UE_SERVER
    
    if (!Task || Task->IsReturnedToPool() || Task->IsTaskRunning()) return;
    
    FScopeLock Lock(&TaskPoolLock);
    Task->Init();
    Task->SetReturnedToPool(true);
    UseItemTaskPool.Enqueue(Task);
    
#endif
    
}