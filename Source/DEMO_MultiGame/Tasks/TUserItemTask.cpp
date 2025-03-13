#include "TUserItemTask.h"
#include "Tables/ItemData.h"
#include "Components/InvenComponent.h"
#include "Characters/PlayerCharacter.h"

void FTUseItemTask::InitializeItemUsage(APlayerCharacter* InPlayer, int32 InItemID)
{
#ifdef UE_SERVER
	PlayerWeak	= InPlayer;
	ItemID		= InItemID;
#endif
}


void FTUseItemTask::DoThreadedWork()
{
#ifdef UE_SERVER
	SetTaskRunning(true);
	APlayerCharacter* Player = PlayerWeak.Get();
	if (!Player || !Player->IsValidLowLevel())
	{
		FinishTask();
		return;
	}

	if (!Player->GetInvenComponent()->GetItemDataList().IsEmpty())
	{
		// find item by ID
		FItemData UsedItem;
		for (const FItemData& Item : Player->GetInvenComponent()->GetItemDataList())
		{
			if (Item.ItemID == ItemID)
			{
				UsedItem = Item;
				break;
			}
		}

		// 아이템 프로퍼티 출력
		UE_LOG(LogTemp, Log, TEXT("Used Item: ID=%d, Name=%s, Type=%llu, Level=%llu, Enhancement=%llu, Durability=%llu, Option=%llu"),
			UsedItem.ItemID, *UsedItem.ItemName, UsedItem.ItemFlags.Type, UsedItem.ItemFlags.Level,
			UsedItem.ItemFlags.Enhancement, UsedItem.ItemFlags.Durability, UsedItem.ItemFlags.Option);

		// 클라이언트에 동기화
		Player->GetInvenComponent()->Multicast_RemoveItemFromList_Implementation(UsedItem.ItemID);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No items to use"));
	}

	FinishTask();
#endif
}


void FTUseItemTask::FinishTask()
{
#ifdef UE_SERVER
	if (bIsTaskRunning)
	{
		bIsTaskRunning = false;
	}
#endif
}
