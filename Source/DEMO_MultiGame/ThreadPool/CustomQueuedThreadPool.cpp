#include "CustomQueuedThreadPool.h"
#include "Thread/CustomThread.h"
#include "Tasks/PoolableQueuedWork.h"


FCustomQueuedThreadPool::FCustomQueuedThreadPool()
{
	WorkAvailableEvent = FPlatformProcess::GetSynchEventFromPool();
}

FCustomQueuedThreadPool::~FCustomQueuedThreadPool()
{
	Destroy();
	
	if (WorkAvailableEvent)
	{
		FPlatformProcess::ReturnSynchEventToPool(WorkAvailableEvent);
		WorkAvailableEvent = nullptr;
	}
}


bool FCustomQueuedThreadPool::Create(const uint32 InNumQueuedThreads, const uint32 StackSize, const EThreadPriority ThreadPriority,
	const TCHAR* Name)
{
	NumThreads = InNumQueuedThreads;
	for (uint32 i = 0; i < InNumQueuedThreads; ++i)
	{
		FCustomThread* NewThread = new FCustomThread(this);
		Threads.Add(NewThread);
		FString ThreadName = FString::Printf(TEXT("%s_Thread%d"), Name, i);
		NewThread->Start(StackSize, ThreadPriority, *ThreadName);
	}
	
	return true;
}


void FCustomQueuedThreadPool::Destroy()
{
	FScopeLock Lock(&SynchronizationObject);
	
	for (FCustomThread* Thread : Threads)
	{
		Thread->Shutdown();
		//delete Thread;
	}
	Threads.Empty();
	NumThreads = 0;

	// 남은 작업 정리
	FPoolableQueuedWork* Work = nullptr;
	while (HighPriorityWork.Dequeue(Work)) delete Work;
	while (NormalPriorityWork.Dequeue(Work)) delete Work;
	while (LowPriorityWork.Dequeue(Work)) delete Work;
}

void FCustomQueuedThreadPool::AddQueuedWork(IQueuedWork* InQueuedWork, EQueuedWorkPriority InQueuedWorkPriority)
{
	FScopeLock Lock(&SynchronizationObject);

	// downcast InQueueWork to FPoolableQueuedWork
	FPoolableQueuedWork* InQueuePoolableWork = static_cast<FPoolableQueuedWork*>(InQueuedWork);
	
	switch(InQueuedWorkPriority)
	{
	case EQueuedWorkPriority::High:
		HighPriorityWork.Enqueue(InQueuePoolableWork);
		break;
	case EQueuedWorkPriority::Low:
		LowPriorityWork.Enqueue(InQueuePoolableWork);
		break;
	default:
		NormalPriorityWork.Enqueue(InQueuePoolableWork);
		break;
	}
    
	// 작업 추가 시 대기 스레드 깨우기 (이벤트 기반)
	WorkAvailableEvent->Trigger();
}

bool FCustomQueuedThreadPool::RetractQueuedWork(IQueuedWork* InQueuedWork)
{
	FScopeLock Lock(&SynchronizationObject);
	TArray<FPoolableQueuedWork*> TempQueue;
	FPoolableQueuedWork* Work = nullptr;
	bool bFound = false;

	for (auto& Queue : { &HighPriorityWork, &NormalPriorityWork, &LowPriorityWork })
	{
		// 큐에서 작업을 찾아 제거
		while (Queue->Dequeue(Work))
		{
			if (Work == InQueuedWork)
			{
				bFound = true;
				delete Work;
			}
			else
			{
				TempQueue.Add(Work);
			}
		}
		
		// 남은 작업 다시 큐에 넣기
		for (FPoolableQueuedWork* RemainingWork : TempQueue)
		{
			Queue->Enqueue(RemainingWork);
		}
		TempQueue.Empty();
	}
	
	return bFound;
}

void FCustomQueuedThreadPool::WaitForCompletion()
{
	while (ActiveTaskCounter.GetValue() > 0)
	{
		FPlatformProcess::Sleep(0.01f);
	}

	FScopeLock Lock(&SynchronizationObject);
	TArray<FPoolableQueuedWork*> AllTasks;
	FPoolableQueuedWork* Work;
	while (HighPriorityWork.Dequeue(Work)) AllTasks.Add(Work);
	while (NormalPriorityWork.Dequeue(Work)) AllTasks.Add(Work);
	while (LowPriorityWork.Dequeue(Work)) AllTasks.Add(Work);

	for (FPoolableQueuedWork* Task : AllTasks)
	{
		if (Task)
		{
			while (Task->IsTaskRunning())
			{
				FPlatformProcess::Sleep(0.01f); // AsyncTask 완료 대기
			}
			delete Task;
		}
	}
}

bool FCustomQueuedThreadPool::AddThread(uint32 StackSize, EThreadPriority ThreadPriority, const TCHAR* Name)
{
	FScopeLock Lock(&SynchronizationObject);
	
	FCustomThread* NewThread = new FCustomThread(this);
	Threads.Add(NewThread);
	
	FString ThreadName = FString::Printf(TEXT("%s_Thread%d"), Name, NumThreads);
	NewThread->Start(StackSize, ThreadPriority, *ThreadName);
	
	NumThreads++;
	
	return true;
}

bool FCustomQueuedThreadPool::RemoveThread()
{
	FScopeLock Lock(&SynchronizationObject);
	if (NumThreads <= 1)
		return false;  // 최소 하나의 스레드는 유지

	const int32 LastIndex = Threads.Num() - 1;
	FCustomThread* Thread = Threads[LastIndex];
	
	Thread->Shutdown();
	delete Thread;
	
	Threads.RemoveAt(LastIndex);
	NumThreads--;
	
	return true;
}
