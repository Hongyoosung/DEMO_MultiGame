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

    // 스레드 풀 생성
    virtual bool Create(const uint32 InNumQueuedThreads, const uint32 StackSize, const EThreadPriority ThreadPriority, const TCHAR* Name) override;

    // 스레드 풀 종료
    virtual void Destroy() override;

    // 작업 추가
    virtual void AddQueuedWork(IQueuedWork* InQueuedWork, EQueuedWorkPriority InQueuedWorkPriority) override;

    // 작업 제거
    virtual bool RetractQueuedWork(IQueuedWork* InQueuedWork) override;
    
    // 스레드 수 반환
    virtual int32 GetNumThreads() const override {  return NumThreads;  }

    void WaitForCompletion();
    void ForceShutDown();
    
    bool AddThread(uint32 StackSize, EThreadPriority ThreadPriority, const TCHAR* Name);
    void IncrementActiveTaskCount   ()      {       ActiveTaskCounter.Increment();      }
    void DecrementActiveTaskCount   ()      {       ActiveTaskCounter.Decrement();      }
    bool RemoveThread();
    

    TQueue<FPoolableQueuedWork*, EQueueMode::Mpsc>& GetHighPriorityWork     ()          {   return HighPriorityWork;        }
    TQueue<FPoolableQueuedWork*, EQueueMode::Mpsc>& GetNormalPriorityWork   ()          {   return NormalPriorityWork;      }
    TQueue<FPoolableQueuedWork*, EQueueMode::Mpsc>& GetLowPriorityWork      ()          {   return LowPriorityWork;         }
    FCriticalSection& GetSynchronizationObject                              ()          {   return SynchronizationObject;   }
    FEvent* GetWorkAvailableEvent                                           ()  const   {   return WorkAvailableEvent;      }
    
    int32                                           GetActiveTaskCount      ()  const   {   return ActiveTaskCounter.GetValue();    }
    

private:
    TArray<FCustomThread*>                          Threads;                // 스레드 배열

    TQueue<FPoolableQueuedWork*, EQueueMode::Mpsc>  HighPriorityWork;
    TQueue<FPoolableQueuedWork*, EQueueMode::Mpsc>  NormalPriorityWork;
    TQueue<FPoolableQueuedWork*, EQueueMode::Mpsc>  LowPriorityWork;
    
    FCriticalSection                                SynchronizationObject;  // 동기화 객체
    
    FThreadSafeCounter                              ActiveTaskCounter;      // 활성 작업 카운터
    FThreadSafeBool                                 bIsDestroying;
    
    int32                                           NumThreads = 0;         // 스레드 수

    FEvent*                                         WorkAvailableEvent;
};
