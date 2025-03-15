#pragma once

#include "CoreMinimal.h"
#include "Misc/QueuedThreadPool.h"


class FPoolableQueuedWork;
class FCustomThread;


class FCustomQueuedThreadPool final : public FQueuedThreadPool
{
public:
    static FCustomQueuedThreadPool* Allocate()
    {
        return new FCustomQueuedThreadPool();
    }

    
    FCustomQueuedThreadPool();
    
    virtual ~FCustomQueuedThreadPool() override;
    
    virtual bool Create(const uint32 InNumQueuedThreads, const uint32 StackSize,
                        const EThreadPriority ThreadPriority, const TCHAR* Name)
                        override;
    
    virtual void Destroy() override;

    
    // Add a task to the thread pool
    virtual void AddQueuedWork(IQueuedWork* InQueuedWork, EQueuedWorkPriority InQueuedWorkPriority) override;

    
    // Remove a task from the thread pool
    virtual bool RetractQueuedWork(IQueuedWork* InQueuedWork) override;

    
    virtual FORCEINLINE int32 GetNumThreads() const override {  return NumThreads;  }

    
    void WaitForCompletion(); // Wait for all tasks to complete
    void ForceShutDown();     // Force shutdown of all tasks


    void IncrementActiveTaskCount   ()          {    ActiveTaskCounter.Increment();  }
    void DecrementActiveTaskCount   ()          {    ActiveTaskCounter.Decrement();  }

    
    bool AddThread(uint32 StackSize, EThreadPriority ThreadPriority, const TCHAR* Name);
    bool RemoveThread();
    

    FORCEINLINE TQueue<FPoolableQueuedWork*, EQueueMode::Mpsc>&
        GetHighPriorityWork         ()          {    return HighPriorityWork;       }
    FORCEINLINE TQueue<FPoolableQueuedWork*, EQueueMode::Mpsc>&
        GetNormalPriorityWork       ()          {   return NormalPriorityWork;      }
    FORCEINLINE TQueue<FPoolableQueuedWork*, EQueueMode::Mpsc>&
        GetLowPriorityWork          ()          {   return LowPriorityWork;         }

    
    FORCEINLINE FCriticalSection&
        GetSynchronizationObject    ()          {   return SynchronizationObject;   }
    FORCEINLINE FEvent*
        GetWorkAvailableEvent       ()  const   {   return WorkAvailableEvent;      }


    
    FORCEINLINE int32 GetActiveTaskCount()  const  {   return ActiveTaskCounter.GetValue();    }
    

private:
    TArray<FCustomThread*>                          Threads;                

    TQueue<FPoolableQueuedWork*, EQueueMode::Mpsc>  HighPriorityWork;
    TQueue<FPoolableQueuedWork*, EQueueMode::Mpsc>  NormalPriorityWork;
    TQueue<FPoolableQueuedWork*, EQueueMode::Mpsc>  LowPriorityWork;
    
    FCriticalSection                                SynchronizationObject;  // sync object
    
    FThreadSafeCounter                              ActiveTaskCounter;      // active task counter
    FThreadSafeBool                                 bIsDestroying;
    
    int32                                           NumThreads = 0;         

    FEvent*                                         WorkAvailableEvent;
};
