#include "CustomQueuedThreadPool.h"
#include "Thread/CustomThread.h"
#include "Tasks/PoolableQueuedWork.h"
#include "DEMO_MultiGame.h"


FCustomQueuedThreadPool::FCustomQueuedThreadPool() : bIsDestroying(false)
{
	WorkAvailableEvent = FPlatformProcess::GetSynchEventFromPool();
}

FCustomQueuedThreadPool::~FCustomQueuedThreadPool()
{
	if (!bIsDestroying) Destroy();
	
	if (WorkAvailableEvent)
	{
		FPlatformProcess::ReturnSynchEventToPool(WorkAvailableEvent);
		WorkAvailableEvent = nullptr;
	}
}


bool FCustomQueuedThreadPool::Create(const uint32 InNumQueuedThreads, const uint32 StackSize, const EThreadPriority ThreadPriority,
	const TCHAR* Name)
{
	FScopeLock Lock(&SynchronizationObject);
	bool bWasSuccessful = true;
	
	
	for (uint32 i = 0; i < InNumQueuedThreads; ++i)
	{
		FCustomThread* NewThread = new FCustomThread(this);
		if (NewThread != nullptr)
		{
			FString ThreadName = FString::Printf(TEXT("%s_Thread%d"), Name, i);
			NewThread->Start(StackSize, ThreadPriority, *ThreadName);
			Threads.Add(NewThread);
			NumThreads++;
		}
		else
		{
			delete NewThread;
			bWasSuccessful = false;
		}
	}
	
	return bWasSuccessful;
}


void FCustomQueuedThreadPool::Destroy()
{
	bIsDestroying = true;

	{
		FScopeLock Lock(&SynchronizationObject);
		FPoolableQueuedWork* Work = nullptr;

		// 모든 우선순위 큐 비우기
		int32 HighPriorityCleared = 0;
		while (HighPriorityWork.Dequeue(Work)) 
		{
			if (Work)
			{
				Work->Abandon();  // 작업 중단 알림
				delete Work;
			}
			HighPriorityCleared++;
		}
		
		int32 NormalPriorityCleared = 0;
		while (NormalPriorityWork.Dequeue(Work)) 
		{
			if (Work)
			{
				Work->Abandon();
				delete Work;
			}
			NormalPriorityCleared++;
		}
		
		int32 LowPriorityCleared = 0;
		while (LowPriorityWork.Dequeue(Work)) 
		{
			if (Work)
			{
				Work->Abandon();
				delete Work;
			}
			LowPriorityCleared++;
		}
		
		// 로그에 남은 작업 수 기록 (디버깅 목적)
		if (HighPriorityCleared > 0 || NormalPriorityCleared > 0 || LowPriorityCleared > 0)
		{
			TESTLOG(Warning, TEXT("Cleared pending tasks: High=%d, Normal=%d, Low=%d"), 
				HighPriorityCleared, NormalPriorityCleared, LowPriorityCleared);
		}
	}

	// 2. 모든 스레드 깨우기 (종료 플래그가 이미 설정됨)
	WorkAvailableEvent->Trigger();
	
	// 3. 스레드 종료 - 각 스레드가 더 이상 작업을 받지 않도록
	{
		FScopeLock Lock(&SynchronizationObject);
		
		// 모든 스레드 종료
		for (FCustomThread* Thread : Threads)
		{
			if (Thread)
			{
				Thread->Shutdown();
				delete Thread;
			}
		}
		
		Threads.Empty();
		NumThreads = 0;
	}
	
	// 4. 활성 작업 카운터 초기화
	if (ActiveTaskCounter.GetValue() > 0)
	{
		TESTLOG(Warning, TEXT("Destroying thread pool with %d active tasks"), ActiveTaskCounter.GetValue());
		ActiveTaskCounter.Reset();
	}
}

void FCustomQueuedThreadPool::AddQueuedWork(IQueuedWork* InQueuedWork, EQueuedWorkPriority InQueuedWorkPriority)
{
	if (bIsDestroying)
	{
		// 종료 중인 경우 작업 거부 및 정리
		TESTLOG(Warning, TEXT("Rejecting work while thread pool is being destroyed"));
		delete InQueuedWork;
		return;
	}
	
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
	const float TimeoutSeconds = 3.0f; // 최대 대기 시간 설정
	float ElapsedTime = 0.0f;
	const float SleepInterval = 0.01f;
	
	// 남은 작업이 있는지 확인
	{
		FScopeLock Lock(&SynchronizationObject);
		
		FPoolableQueuedWork* Work = nullptr;
		
		if (!HighPriorityWork.IsEmpty() || !NormalPriorityWork.IsEmpty() || LowPriorityWork.IsEmpty() || ActiveTaskCounter.GetValue() > 0)
		{
			TESTLOG(Log, TEXT("Waiting for %d queued tasks and %d active tasks to complete"), 
				ActiveTaskCounter.GetValue());
		}
	}
	
	while (ActiveTaskCounter.GetValue() > 0 && ElapsedTime < TimeoutSeconds)
	{
		FPlatformProcess::Sleep(SleepInterval);
		ElapsedTime += SleepInterval;
		
		// 주기적으로 로그 출력 (500ms 마다)
		if (FMath::FloorToInt(ElapsedTime / 0.5f) != FMath::FloorToInt((ElapsedTime - SleepInterval) / 0.5f))
		{
			TESTLOG(Log, TEXT("Still waiting for completion: %d active tasks, %.1f seconds elapsed"), 
				ActiveTaskCounter.GetValue(), ElapsedTime);
		}
	}
	
	if (ActiveTaskCounter.GetValue() > 0)
	{
		TESTLOG(Warning, TEXT("WaitForCompletion timed out after %.1f seconds with %d active tasks"),
			TimeoutSeconds, ActiveTaskCounter.GetValue());
		ForceShutDown();
	}
	else
	{
		TESTLOG(Log, TEXT("All tasks completed successfully"));
	}
}

void FCustomQueuedThreadPool::ForceShutDown()
{
	FScopeLock Lock(&SynchronizationObject);
	
	// 모든 스레드 종료
	for (FCustomThread* Thread : Threads)
	{
		Thread->Shutdown();
		delete Thread;
	}
	
	Threads.Empty();
	NumThreads = 0;

	// 남은 태스크 강제 종료 및 삭제
	FPoolableQueuedWork* Work;
	while (HighPriorityWork.Dequeue(Work))
	{
		if (Work)
		{
			Work->Abandon();
			delete Work;
		}
	}
	while (NormalPriorityWork.Dequeue(Work))
	{
		if (Work)
		{
			Work->Abandon();
			delete Work;
		}
	}
	while (LowPriorityWork.Dequeue(Work))
	{
		if (Work)
		{
			Work->Abandon();
			delete Work;
		}
	}
	ActiveTaskCounter.Reset(); // 활성 태스크 카운터 초기화
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
