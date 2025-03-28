﻿#pragma once

#include "CoreMinimal.h"
#include "Async/AsyncWork.h"
#include "Tables/ItemData.h"
#include "Tasks/PoolableQueuedWork.h"


class APlayerCharacter;


class FTAcquireItemTask final : public FPoolableQueuedWork
{
public:
	void InitializeAcquireItem(APlayerCharacter* InPlayer, const FItemData& InItem);

	void SetCompletionCallback(TFunction<void(FTAcquireItemTask*)> InCallback);

	virtual void DoThreadedWork() override;
	virtual void Abandon() override {}
	virtual void Init() override {}

	
private:
	void FinishTask();

	TFunction<void(FTAcquireItemTask*)> CompletionCallback;
	TWeakObjectPtr<APlayerCharacter>	PlayerWeak;
	
	FItemData ItemData;
};