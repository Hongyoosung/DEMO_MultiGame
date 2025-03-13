#include "InvenComponent.h"
#include "Net/UnrealNetwork.h"
#include "Tables/ItemData.h"
#include "GameModes/MultiGameMode.h"
#include "Characters/PlayerCharacter.h"
#include "Tasks/TAcquireItemTask.h"
#include "DEMO_MultiGame.h"



UInvenComponent::UInvenComponent() : OwnerCharacter(nullptr), GameMode(nullptr)
{
}


void UInvenComponent::AcquireItem()
{
	Client_AcquireItem();
}


void UInvenComponent::UseItem(const int32 ItemID)
{
	Client_UseItem(ItemID);
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
	
	GameMode = Cast<AMultiGameMode>(GetWorld()->GetAuthGameMode());
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


bool UInvenComponent::Server_UseItem_Validate(const int32 ItemID)
{
#ifdef UE_SERVER
	if (!OwnerCharacter || OwnerCharacter->ItemVerification(OwnerCharacter, ItemID))
	{
		TESTLOG(Warning, TEXT("Failed to get GameMode or AntiCheatManager"));
		return false;
	}

	return true;
#endif
}


bool UInvenComponent::Server_AcquireItem_Validate()
{
	return true;
}


void UInvenComponent::Server_AcquireItem_Implementation()
{
#ifdef UE_SERVER
	if (!OwnerCharacter)
	{
		TESTLOG(Warning, TEXT("Checksum verification failed for player %s"), *GetName());
		return;
	}
    

	if (IsValid(OwnerCharacter))
	{
		TESTLOG(Error, TEXT("Invalid ThreadPool or Player"));
		return;
	}

	
	FTAcquireItemTask* Task = OwnerCharacter->GetGameMode()->GetOrCreateAcquireItemTask();
	if (!Task)
	{
		TESTLOG(Error, TEXT("Failed to get or create attack task"));
		return;
	}
	
	OwnerCharacter->GetGameMode()->ExecuteAcquireItemTask(Task);

#endif
}


void UInvenComponent::Server_UseItem_Implementation(const int32 UseItemID)
{
#ifdef UE_SERVER
	
	if (!OwnerCharacter)
	{
		TESTLOG(Warning, TEXT("Checksum verification failed for player %s"), *GetName());
		return;
	}
    

	if (IsValid(OwnerCharacter))
	{
		TESTLOG(Error, TEXT("Invalid ThreadPool or Player"));
		return;
	}

	
	FTUseItemTask* Task = OwnerCharacter->GetGameMode()->GetOrCreateUseItemTask();
	if (!Task)
	{
		TESTLOG(Error, TEXT("Failed to get or create attack task"));
		return;
	}
	
	OwnerCharacter->GetGameMode()->ExecuteUseItemTask(Task);

#endif
}


void UInvenComponent::Client_AcquireItem()
{
	Server_AcquireItem();
}

void UInvenComponent::Client_UseItem(const int32 ItemID)
{
	Server_UseItem(ItemID);
}


void UInvenComponent::Multicast_AddItemToList_Implementation(FItemData NewItem)
{
	ItemList.Add(NewItem);
}


void UInvenComponent::Multicast_RemoveItemFromList_Implementation(int32 RemovedItemID)
{
	if (FItemData* Item = ItemList.FindByPredicate
		([RemovedItemID](FItemData& Item)
			{ return Item.ItemID == RemovedItemID; }))
	{
		ItemList.Remove(*Item);
	}
}


