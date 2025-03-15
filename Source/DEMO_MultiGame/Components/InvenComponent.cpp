#include "InvenComponent.h"
#include "Net/UnrealNetwork.h"
#include "Tables/ItemData.h"
#include "GameModes/MultiGameMode.h"
#include "Characters/PlayerCharacter.h"
#include "Tasks/TAcquireItemTask.h"
#include "DEMO_MultiGame.h"
#include "Tasks/TUseItemTask.h"


UInvenComponent::UInvenComponent() : OwnerCharacter(nullptr), GameMode(nullptr)
{
	SetIsReplicatedByDefault(true);
}


void UInvenComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<APlayerCharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		TESTLOG(Warning, TEXT("InvenComponent not attached to PlayerCharacter"));
		return;
	}

	TESTLOG(Display, TEXT("InvenComponent BeginPlay - OwnerRole: %d, RemoteRole: %d, Component Address: %p"),
		GetOwnerRole(), GetOwner()->GetRemoteRole(), this);
}


void UInvenComponent::InitializeGameMode(AMultiGameMode* InGameMode)
{
	GameMode = InGameMode;

	if (!GameMode)
	{
		TESTLOG(Warning, TEXT("Failed to get MultiGameMode"));
		return;
	}
}


void UInvenComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UInvenComponent, ItemList);
}


void UInvenComponent::RequestAcquireItem()
{
	Client_OnItemAcquired();
}


void UInvenComponent::RequestUseItem(const int32 ItemID)
{
	Client_OnItemUsed(ItemID);
}


void UInvenComponent::Client_OnItemAcquired_Implementation()
{
	Server_RequestAcquireItem();
}


void UInvenComponent::Client_OnItemUsed_Implementation(int32 ItemID)
{
	// 아이템이 클라이언트 목록에 있는지 확인
	bool bItemExists = false;
	for (const FItemData& Item : ItemList)
	{
		if (Item.ItemID == ItemID)
		{
			bItemExists = true;
			break;
		}
	}

	if (bItemExists)
	{
		// 클라이언트에서 서버로 RPC 호출
		Server_RequestUseItem(ItemID);
	}
	else
	{
		TESTLOG(Warning, TEXT("Client tried to use non-existent item: %d"), ItemID);
	}
}

void UInvenComponent::ProcessItemAcquisition(const FItemData& ItemData)
{
	ItemList.Add(ItemData);
	TESTLOG(Display, TEXT("Multicast_AddItemToList - Component Address: %p, Item Added: %s, Total Items: %d"), 
		this, *ItemData.ItemName, ItemList.Num());
}

void UInvenComponent::ProcessItemUsage(int32 ItemID)
{
	int32 Index = -1;
	for (int32 i = 0; i < ItemList.Num(); ++i)
	{
		if (ItemList[i].ItemID == ItemID)
		{
			Index = i;
			break;
		}
	}

	if (Index != -1)
	{
		ItemList.RemoveAt(Index);
		TESTLOG(Display, TEXT("Multicast_RemoveItemFromList - Component Address: %p, Item Removed: %d, Total Items: %d"), 
			this, ItemID, ItemList.Num());
	}
	else
	{
		TESTLOG(Warning, TEXT("Failed to remove item: %d, not found in list"), ItemID);
	}
}


bool UInvenComponent::Server_RequestUseItem_Validate(const int32 ItemID)
{
	if (!OwnerCharacter)
	{
		TESTLOG(Warning, TEXT("Invalid OwnerCharacter in Server_UseItem_Validate"));
		return false;
	}
	
	if (!OwnerCharacter->ItemVerification(OwnerCharacter, ItemID))
	{
		TESTLOG(Warning, TEXT("Item verification failed for ItemID: %d"), ItemID);
		return false;
	}

	return true;
}


bool UInvenComponent::Server_RequestAcquireItem_Validate()
{
	if (!OwnerCharacter)
	{
		TESTLOG(Warning, TEXT("Invalid OwnerCharacter in Server_AcquireItem_Validate"));
		return false;
	}
    
	return true;
}


void UInvenComponent::Server_RequestAcquireItem_Implementation()
{
#ifdef UE_SERVER
	if (!OwnerCharacter || !GameMode)
	{
		TESTLOG(Error, TEXT("Invalid GameMode or Player"));
		return;
	}

	
	FTAcquireItemTask* Task = GameMode->GetOrCreateAcquireItemTask();
	if (!Task)
	{
		TESTLOG(Error, TEXT("Failed to get or create attack task"));
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

	UE_LOG(LogTemp, Display, TEXT("Created new item: %s (ID: %d, Type: %llu)"), 
		*NewItem.ItemName, NewItem.ItemID, NewItem.ItemFlags.Type);
		
	Task->InitializeAcquireItem(OwnerCharacter, NewItem);
	
	GameMode->ExecuteAcquireItemTask(Task);

#endif
}


void UInvenComponent::Server_RequestUseItem_Implementation(const int32 UseItemID)
{
#ifdef UE_SERVER
	
	if (!OwnerCharacter || !GameMode)
	{
		TESTLOG(Warning, TEXT("Checksum verification failed for player %s"), *GetName());
		return;
	}

	
	FTUseItemTask* Task = GameMode->GetOrCreateUseItemTask();
	if (!Task)
	{
		TESTLOG(Error, TEXT("Failed to get or create attack task"));
		return;
	}

	Task->InitializeItemUsage(OwnerCharacter, UseItemID);
	GameMode->ExecuteUseItemTask(Task);

#endif
}




