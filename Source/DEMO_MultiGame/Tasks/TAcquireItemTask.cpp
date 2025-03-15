#include "TAcquireItemTask.h"

#include "DEMO_MultiGame.h"
#include "Characters/PlayerCharacter.h"
#include "Components/InvenComponent.h"
#include "Tables/ItemData.h"


void FTAcquireItemTask::InitializeAcquireItem(APlayerCharacter* InPlayer, const FItemData& InItem)
{
	PlayerWeak = InPlayer;
	ItemData = InItem;
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


	UInvenComponent* InvenComponent = Player->GetInvenComponent();
	if (!InvenComponent)
	{
		TESTLOG(Error, TEXT("Invalid InvenComponent in AcquireItemTask"));
		FinishTask();
		return;
	}

	
	// 멀티캐스트 호출을 게임 스레드에서 실행
	AsyncTask(ENamedThreads::GameThread, [InvenComponent, ItemData = this->ItemData]()
	{
		if (InvenComponent && InvenComponent->IsValidLowLevel())
		{
			InvenComponent->ProcessItemAcquisition(ItemData);
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
