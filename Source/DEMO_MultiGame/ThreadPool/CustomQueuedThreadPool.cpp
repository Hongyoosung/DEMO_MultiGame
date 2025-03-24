#include "CustomQueuedThreadPool.h"
#include "Thread/CustomThread.h"
#include "Tasks/PoolableQueuedWork.h"
#include "DEMO_MultiGame.h"


FCustomQueuedThreadPool::FCustomQueuedThreadPool() : bIsDestroying(false)
{
	
#ifdef UE_SERVER
	
	WorkAvailableEvent = FPlatformProcess::GetSynchEventFromPool();
	
#endif
	
}


FCustomQueuedThreadPool::~FCustomQueuedThreadPool()
{
	
#ifdef UE_SERVER
	
	if (!bIsDestroying) Destroy();
	
	if (WorkAvailableEvent)
	{
		FPlatformProcess::ReturnSynchEventToPool(WorkAvailableEvent);
		WorkAvailableEvent = nullptr;
	}
	
#endif
	
}


bool FCustomQueuedThreadPool::Create(const uint32 InNumQueuedThreads, const uint32 StackSize, const EThreadPriority ThreadPriority,
	const TCHAR* Name)
{
	
#ifdef UE_SERVER
	
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
			NewThread = nullptr;
			bWasSuccessful = false;
		}
		
	}
	
	return bWasSuccessful;

#endif
	
}


void FCustomQueuedThreadPool::Destroy()
{
	
#ifdef UE_SERVER
	
	bIsDestroying = true;

	// 1. Clear all pending tasks and abandon them
	{
		FScopeLock Lock(&SynchronizationObject);
		FPoolableQueuedWork* Work = nullptr;

		// Clear all pending tasks
		int32 HighPriorityCleared = 0;
		while (HighPriorityWork.Dequeue(Work)) 
		{
			if (Work)
			{
				Work->Abandon();  // Abandon the task
				delete Work;
				Work = nullptr;
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
				Work = nullptr;
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
				Work = nullptr;
			}
			LowPriorityCleared++;
		}
		
		// Record the number of tasks left in the log (for debugging purposes)
		if (HighPriorityCleared > 0 || NormalPriorityCleared > 0 || LowPriorityCleared > 0)
		{
			TESTLOG(Warning, TEXT("Cleared pending tasks: High=%d, Normal=%d, Low=%d"), 
				HighPriorityCleared, NormalPriorityCleared, LowPriorityCleared);
		}
	}

	
	// 2. Wake all threads (exit flag already set)
	WorkAvailableEvent->Trigger();

	
	// 3. Terminate threads so that each thread is no longer receiving work.
	{
		FScopeLock Lock(&SynchronizationObject);
		
		// trun off all threads
		for (FCustomThread* Thread : Threads)
		{
			if (Thread)
			{
				Thread->Shutdown();
				delete Thread;
				Thread = nullptr;
			}
		}
		
		Threads.Empty();
		NumThreads = 0;
	}

	
	// 4. Resetting the active task counter
	if (ActiveTaskCounter.GetValue() > 0)
	{
		TESTLOG(Warning, TEXT("Destroying thread pool with %d active tasks"), ActiveTaskCounter.GetValue());
		ActiveTaskCounter.Reset();
	}
	
#endif
	
}


void FCustomQueuedThreadPool::AddQueuedWork(IQueuedWork* InQueuedWork, EQueuedWorkPriority InQueuedWorkPriority)
{
	
#ifdef UE_SERVER
	
	if (bIsDestroying)
	{
		// Reject and clean up tasks if they are shutting down
		TESTLOG(Warning, TEXT("Rejecting work while thread pool is being destroyed"));
		delete InQueuedWork;
		InQueuedWork = nullptr;
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
    
	// Wake up waiting threads when adding a task (event-driven)
	WorkAvailableEvent->Trigger();

#endif
	
}


bool FCustomQueuedThreadPool::RetractQueuedWork(IQueuedWork* InQueuedWork)
{
	
#ifdef UE_SERVER
	
	FScopeLock Lock(&SynchronizationObject);
	
	TArray<FPoolableQueuedWork*>	TempQueue;
	FPoolableQueuedWork*			Work = nullptr;
	bool							bFound = false;
	

	for (auto& Queue : { &HighPriorityWork, &NormalPriorityWork, &LowPriorityWork })
	{
		// Find and remove tasks from the queue
		while (Queue->Dequeue(Work))
		{
			if (Work == InQueuedWork)
			{
				bFound = true;
				delete Work;
				Work = nullptr;
			}
			else
			{
				TempQueue.Add(Work);
			}
		}
		
		// Requeue remaining work
		for (FPoolableQueuedWork* RemainingWork : TempQueue)
		{
			Queue->Enqueue(RemainingWork);
		}
		TempQueue.Empty();
	}
	
	return bFound;
	
#endif
	
}


void FCustomQueuedThreadPool::WaitForCompletion()
{
	
#ifdef UE_SERVER
	
	const float TimeoutSeconds	= 3.0f; // Setting the maximum wait time
	float		ElapsedTime		= 0.0f;
	const float SleepInterval	= 0.01f;

	
	// Check the remaining tasks and active tasks
	{
		FScopeLock Lock(&SynchronizationObject);
		
		FPoolableQueuedWork* Work = nullptr;
		
		if (!HighPriorityWork.IsEmpty() || !NormalPriorityWork.IsEmpty() || LowPriorityWork.IsEmpty() || ActiveTaskCounter.GetValue() > 0)
		{
			TESTLOG(Log, TEXT("Waiting for %d queued tasks and %d active tasks to complete"), 
				ActiveTaskCounter.GetValue());
		}
	}

	
	// Wait for the active task to complete or reach a timeout
	while (ActiveTaskCounter.GetValue() > 0 && ElapsedTime < TimeoutSeconds)
	{
		FPlatformProcess::Sleep(SleepInterval);
		ElapsedTime += SleepInterval;
		
		// Periodic log output (every 500 ms)
		if (FMath::FloorToInt(ElapsedTime / 0.5f) != FMath::FloorToInt((ElapsedTime - SleepInterval) / 0.5f))
		{
			TESTLOG(Log, TEXT("Still waiting for completion: %d active tasks, %.1f seconds elapsed"), 
				ActiveTaskCounter.GetValue(), ElapsedTime);
		}
	}

	// Processing results after waiting is complete
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
	
#endif
	
}


void FCustomQueuedThreadPool::ForceShutDown()
{
	
#ifdef UE_SERVER
	
	FScopeLock Lock(&SynchronizationObject);
	
	// Terminate all threads
	for (FCustomThread* Thread : Threads)
	{
		Thread->Shutdown();
		delete Thread;
		Thread = nullptr;
	}
	
	Threads.Empty();
	NumThreads = 0;

	
	// Force terminate and delete leftover tasks
	FPoolableQueuedWork* Work;
	
	while (HighPriorityWork.Dequeue(Work))
	{
		if (Work)
		{
			Work->Abandon();
			delete Work;
			Work = nullptr;
		}
	}
	while (NormalPriorityWork.Dequeue(Work))
	{
		if (Work)
		{
			Work->Abandon();
			delete Work;
			Work = nullptr;
		}
	}
	while (LowPriorityWork.Dequeue(Work))
	{
		if (Work)
		{
			Work->Abandon();
			delete Work;
			Work = nullptr;
		}
	}
	
	ActiveTaskCounter.Reset(); // 활성 태스크 카운터 초기화
	
#endif
	
}


bool FCustomQueuedThreadPool::AddThread(uint32 StackSize, EThreadPriority ThreadPriority, const TCHAR* Name)
{
	
#ifdef UE_SERVER
	
	FScopeLock Lock(&SynchronizationObject);
	
	FCustomThread* NewThread = new FCustomThread(this);
	Threads.Add(NewThread);
	
	FString ThreadName = FString::Printf(TEXT("%s_Thread%d"), Name, NumThreads);
	NewThread->Start(StackSize, ThreadPriority, *ThreadName);
	
	NumThreads++;
	
	return true;
	
#endif
	
}


bool FCustomQueuedThreadPool::RemoveThread()
{
	
#ifdef UE_SERVER
	
	FScopeLock Lock(&SynchronizationObject);
	
	if (NumThreads <= 1)
		return false;  // Keep at least one thread

	
	const int32 LastIndex = Threads.Num() - 1;
	FCustomThread* Thread = Threads[LastIndex];

	
	Thread->Shutdown();
	delete Thread;
	Thread = nullptr;

	
	Threads.RemoveAt(LastIndex);
	NumThreads--;
	
	return true;

#endif
	
}
