#include "CustomThread.h"

#include "DEMO_MultiGame.h"
#include "ThreadPool/CustomQueuedThreadPool.h"
#include "Tasks/PoolableQueuedWork.h"

uint32 FCustomThread::Run()
{
	while (!bShutdown)
	{
		FPoolableQueuedWork* Work = nullptr;
		bool bFoundWork = false;
		{
			FScopeLock Lock(&Pool->GetSynchronizationObject());
			if (Pool->GetHighPriorityWork().Dequeue(Work) ||
				Pool->GetNormalPriorityWork().Dequeue(Work) ||
				Pool->GetLowPriorityWork().Dequeue(Work))
			{
				bFoundWork = true;
				Pool->IncrementActiveTaskCount();
			}
		} // 락 해제됨

		if (bFoundWork)
		{
			TESTLOG(Log, TEXT("Executing task %p"), Work);
			Work->DoThreadedWork();
			Pool->DecrementActiveTaskCount();
			TESTLOG(Log, TEXT("Task %p completed"), Work);
		}
		else
		{
			// 락 해제 후 대기: 다른 스레드가 작업을 추가할 수 있음
			Pool->GetWorkAvailableEvent()->Wait();
		}
	}
	return 0;
}


void FCustomThread::Shutdown()
{
	bShutdown = true;
	Pool->GetWorkAvailableEvent()->Trigger();
	if (Thread)
	{
		Thread->WaitForCompletion();
		delete Thread;
		Thread = nullptr;
	}
}
