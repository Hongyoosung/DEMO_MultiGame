#pragma once

#include "CoreMinimal.h"
#include "Misc/QueuedThreadPool.h"
#include "Templates/UniquePtr.h"

class FCustomQueuedThreadPool;


class FCustomThread : public FRunnable
{
public:
    FCustomThread(FCustomQueuedThreadPool* InPool);
    ~FCustomThread() override;

    
    virtual bool    Init()  override { return true; }
    virtual void    Exit()  override {}
    virtual uint32  Run()   override;
    virtual void    Stop()  override { bShutdown = true; }

    
    void Start(const uint32 StackSize, const EThreadPriority Priority, const TCHAR* Name);
    void Shutdown();

    
private:
    FCustomQueuedThreadPool* Pool;
    FThreadSafeBool bShutdown;
    FRunnableThread* Thread;
};