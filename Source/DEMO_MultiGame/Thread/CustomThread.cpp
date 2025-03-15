#include "CustomThread.h"

#include "DEMO_MultiGame.h"
#include "ThreadPool/CustomQueuedThreadPool.h"
#include "Tasks/PoolableQueuedWork.h"


FCustomThread::FCustomThread(FCustomQueuedThreadPool* InPool): Pool(InPool), bShutdown(false), Thread(nullptr)
{
}


FCustomThread::~FCustomThread()
{
}


void FCustomThread::Start(const uint32 StackSize, const EThreadPriority Priority, const TCHAR* Name)
{
#ifdef UE_SERVER
	
	Thread = FRunnableThread::Create(this, Name, StackSize, Priority);
	
#endif
	
}


uint32 FCustomThread::Run()
{
	
#ifdef UE_SERVER
	
	while (!bShutdown)
	{
		FPoolableQueuedWork* Work = nullptr;
		bool bFoundWork = false;

		
		// Pulling tasks out of a pool
		{
			FScopeLock Lock(&Pool->GetSynchronizationObject());
			if (Pool->GetHighPriorityWork	().Dequeue(Work) ||
				Pool->GetNormalPriorityWork	().Dequeue(Work) ||
				Pool->GetLowPriorityWork	().Dequeue(Work))
			{
				bFoundWork = true;
				Pool->IncrementActiveTaskCount();
			}
		} 


		// Run the ejected task
		if (bFoundWork && Work)
		{
			TESTLOG(Log, TEXT("Executing task %p"), Work);
			
			try
			{
				Work->DoThreadedWork();
			}
			catch (const std::exception& e)
			{
				TESTLOG(Error, TEXT("Exception in thread pool: %s"), UTF8_TO_TCHAR(e.what()));
			}
			catch (...)
			{
				TESTLOG(Error, TEXT("Unknown exception in thread pool"));
			}

			
			Pool->DecrementActiveTaskCount();
			
			TESTLOG(Log, TEXT("Task %p completed"), Work);
		}
		else if (bFoundWork && !Work)
		{
			// Work is null
			TESTLOG(Error, TEXT("Dequeued null work from pool"));
			Pool->DecrementActiveTaskCount();
		}
		else
		{
			// Wait after unlocking: Allows other threads to add tasks
			Pool->GetWorkAvailableEvent()->Wait();
		}
	}
	
	return 0;

#endif
	
}


void FCustomThread::Shutdown()
{
	
#ifdef UE_SERVER
	
	bShutdown = true;
	
	Pool->GetWorkAvailableEvent()->Trigger();
	if (Thread)
	{
		Thread->WaitForCompletion();
		delete Thread;
		Thread = nullptr;
	}
	
#endif
	
}
