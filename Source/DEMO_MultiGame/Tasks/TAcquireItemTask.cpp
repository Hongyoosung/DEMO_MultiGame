#include "TAcquireItemTask.h"

#include "DEMO_MultiGame.h"
#include "Characters/PlayerCharacter.h"
#include "Components/InvenComponent.h"
#include "Tables/ItemData.h"


void FTAcquireItemTask::InitializeAcquireItem(APlayerCharacter* InPlayer, const FItemData& InItem)
{
	PlayerWeak	= InPlayer;
	ItemData	= InItem;
}


void FTAcquireItemTask::SetCompletionCallback(TFunction<void(FTAcquireItemTask*)> InCallback)
{
	
#ifdef UE_SERVER
	
	CompletionCallback = MoveTemp(InCallback);
	
#endif
	
}


void FTAcquireItemTask::DoThreadedWork()
{
	
#ifdef UE_SERVER
	
	SetTaskRunning(true);
	
	APlayerCharacter* Player = PlayerWeak.Get();
	if (!Player || !Player->IsValidLowLevel())
	{
		TESTLOG(Error, TEXT("Invalid Player in AcquireItemTask"));
		FinishTask();
		return;
	}

	
	// Execute the sync call on the game thread
	AsyncTask(ENamedThreads::GameThread, [Player, ItemData = this->ItemData]()
	{
		if (Player && Player->IsValidLowLevel())
		{
			Player->TakeAcquireItem(ItemData);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid InvenComponent during AsyncTask execution."));
		}
	});

	FinishTask();
	
#endif
	
}


void FTAcquireItemTask::FinishTask()
{
	
#ifdef UE_SERVER
	
	if (bIsTaskRunning)
	{
		bIsTaskRunning = false;
	}
	
#endif
	
}
