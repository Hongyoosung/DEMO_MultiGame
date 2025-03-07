#pragma once

#include "CoreMinimal.h"
#include "Misc/QueuedThreadPool.h"
#include "Templates/UniquePtr.h"

class FCustomQueuedThreadPool;


class FCustomThread : public FRunnable
{
public:
    FCustomThread(FCustomQueuedThreadPool* InPool)
        : Pool(InPool), bShutdown(false), Thread(nullptr)
    {}

    
    virtual bool    Init()  override { return true; }
    virtual void    Exit()  override {}
    virtual uint32  Run()   override;

    
    void Start(const uint32 StackSize, const EThreadPriority Priority, const TCHAR* Name)
    {
        Thread = FRunnableThread::Create(this, Name, StackSize, Priority);
    }

    void Shutdown();

    
private:
    FCustomQueuedThreadPool* Pool;
    FThreadSafeBool bShutdown;
    FRunnableThread* Thread;
};