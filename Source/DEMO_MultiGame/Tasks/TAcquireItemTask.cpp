#include "TAcquireItemTask.h"
#include "Characters/PlayerCharacter.h"
#include "Components/InvenComponent.h"
#include "Tables/ItemData.h"


void FTAcquireItemTask::InitializeAcquireItem(APlayerCharacter* InPlayer)
{
	PlayerWeak = InPlayer;
}


void FTAcquireItemTask::DoThreadedWork()
{
#ifdef UE_SERVER
	SetTaskRunning(true);
	APlayerCharacter* Player = PlayerWeak.Get();
	if (!Player || !Player->IsValidLowLevel())
	{
		FinishTask();
		return;
	}

	// 랜덤 아이템 생성
	FItemData NewItem;
	NewItem.ItemID					= FMath::RandRange(1, 100);
	NewItem.ItemName				= FString::Printf(TEXT("Item %d"), NewItem.ItemID);
	NewItem.ItemFlags.Type			= FMath::RandRange(0, 255);
	NewItem.ItemFlags.Level			= FMath::RandRange(0, 15);
	NewItem.ItemFlags.Enhancement	= FMath::RandRange(0, 15);
	NewItem.ItemFlags.Durability	= FMath::RandRange(0, 1023);
	NewItem.ItemFlags.Option		= FMath::RandRange(0, 63);
	NewItem.ItemFlags.Reserved		= 0;

	UE_LOG(LogTemp, Display, TEXT("NewItem %s"), *NewItem.ItemName);

	// 플레이어의 아이템 큐에 추가
	Player->GetInvenComponent()->GetItemDataList().Add(NewItem);

	Player->GetInvenComponent()->Multicast_AddItemToList_Implementation(NewItem);

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
