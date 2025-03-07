#pragma once

class FPoolableQueuedWork : public IQueuedWork
{
public:
	virtual ~FPoolableQueuedWork() override {}
    
	virtual void Init() = 0;
    
	bool IsReturnedToPool() const { return bReturnedToPool; }
	bool IsTaskRunning() const { return bIsTaskRunning; }
	void SetReturnedToPool(const bool bInReturnedToPool) { bReturnedToPool = bInReturnedToPool; }

protected:
	bool bReturnedToPool = false;
	FThreadSafeBool bIsTaskRunning;
};
