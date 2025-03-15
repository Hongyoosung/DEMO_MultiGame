#pragma once

#include "CoreMinimal.h"
#include "Async/AsyncWork.h"
#include "Tasks/PoolableQueuedWork.h"

struct FItemData;
class APlayerCharacter;

class FTUseItemTask final : public FPoolableQueuedWork
{
public:
	void			InitializeItemUsage(APlayerCharacter* InPlayer, int32 InItemID);
	void			SetCompletionCallback(TFunction<void(FTUseItemTask*)> InCallback);


	virtual void	DoThreadedWork	() override;
	virtual void	Abandon			() override {};
	virtual void	Init			() override {};

	
private:
	void FinishTask();
	void ApplyItemEffect(const FItemData& Item, APlayerCharacter* Player);

	
private:
	TWeakObjectPtr<APlayerCharacter>	PlayerWeak;
	TFunction<void(FTUseItemTask*)>		CompletionCallback;
	
	int32 ItemID = 0;
};
