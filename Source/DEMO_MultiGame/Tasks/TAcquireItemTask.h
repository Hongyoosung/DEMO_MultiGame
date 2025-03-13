#pragma once

#include "CoreMinimal.h"
#include "Async/AsyncWork.h"
#include "Tasks/PoolableQueuedWork.h"


class APlayerCharacter;


class FTAcquireItemTask final : public FPoolableQueuedWork
{
public:
	void InitializeAcquireItem(APlayerCharacter* InPlayer);

	void SetCompletionCallback(TFunction<void(FTAcquireItemTask*)> InCallback)
	{
#ifdef UE_SERVER
		CompletionCallback = MoveTemp(InCallback);
#endif
	}

	virtual void DoThreadedWork() override;
	virtual void Abandon() override {}
	virtual void Init() override {}

private:
	void FinishTask();

	TWeakObjectPtr<APlayerCharacter> PlayerWeak;
	TFunction<void(FTAcquireItemTask*)> CompletionCallback;
};