#include "TUseItemTask.h"
#include "DEMO_MultiGame.h"
#include "Tables/ItemData.h"
#include "Characters/PlayerCharacter.h"

void FTUseItemTask::InitializeItemUsage(APlayerCharacter* InPlayer, int32 InItemID)
{
	PlayerWeak	= InPlayer;
	ItemID		= InItemID;
}


void FTUseItemTask::SetCompletionCallback(TFunction<void(FTUseItemTask*)> InCallback)
{
	
#ifdef UE_SERVER
	
	CompletionCallback = MoveTemp(InCallback);
	
#endif
	
}


void FTUseItemTask::DoThreadedWork()
{
#ifdef UE_SERVER
	SetTaskRunning(true);
	
	APlayerCharacter* Player = PlayerWeak.Get();
	if (!Player || !Player->IsValidLowLevel())
	{
		TESTLOG(Error, TEXT("Invalid Player in UseItemTask"));
		FinishTask();
		return;
	}
	

	// 아이템 찾기
	FItemData* FoundItem = nullptr;
	for (FItemData& Item : Player->GetItemList())
	{
		if (Item.ItemID == ItemID)
		{
			FoundItem = &Item;
			break;
		}
	}

	if (FoundItem)
	{
		// 아이템 사용 효과 처리
		UE_LOG(LogTemp, Log, TEXT("Using Item: ID=%d, Name=%s, Type=%llu, Level=%llu, Enhancement=%llu, Durability=%llu"),
			FoundItem->ItemID, *FoundItem->ItemName, FoundItem->ItemFlags.Type, FoundItem->ItemFlags.Level,
			FoundItem->ItemFlags.Enhancement, FoundItem->ItemFlags.Durability);
		
		ApplyItemEffect(*FoundItem, Player);
		
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Item with ID %d not found in player's inventory"), ItemID);
	}

	
	// 멀티캐스트 호출을 게임 스레드에서 실행
	AsyncTask(ENamedThreads::GameThread, [Player, FoundItem]()
	{
		if (Player && Player->IsValidLowLevel())
		{
			// 클라이언트들에게 아이템 제거 알림
			Player->TakeUseItem(FoundItem->ItemID);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid InvenComponent during AsyncTask execution."));
		}
	});

	FinishTask();
#endif
}


void FTUseItemTask::ApplyItemEffect(const FItemData& Item, APlayerCharacter* Player)
{
	// Implementing effects based on item type
	switch (Item.ItemFlags.Type)
	{
	case 0: // Health Item
			// Player->GetHealthComponent()->Heal(Item.ItemFlags.Level * 10.0f);
			TESTLOG(Warning, TEXT("Applied healing effect from item %s"), *Item.ItemName);
		break;
		
	case 1: // Increase Attack Item
			// Player->BoostAttack(Item.ItemFlags.Level * 5.0f, 10.0f);
			TESTLOG(Warning, TEXT("Applied attack boost from item %s"), *Item.ItemName);
		break;
		
	case 2: // Increase Defense Item
			// Player->BoostDefense(Item.ItemFlags.Level * 5.0f, 10.0f);
			TESTLOG(Warning, TEXT("Applied defense boost from item %s"), *Item.ItemName);
		break;
		
	case 3: // Increase Speed Item
			// Player->BoostSpeed(Item.ItemFlags.Level * 0.1f, 8.0f);
			TESTLOG(Warning, TEXT("Applied speed boost from item %s"), *Item.ItemName);
		break;
		
	default:
		TESTLOG(Warning, TEXT("Item %s has no specific effect"), *Item.ItemName);
		break;
	}
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