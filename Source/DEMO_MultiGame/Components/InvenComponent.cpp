#include "InvenComponent.h"
#include "Net/UnrealNetwork.h"
#include "Tables/ItemData.h"
#include "GameModes/MultiGameMode.h"
#include "Characters/PlayerCharacter.h"
#include "Tasks/TAcquireItemTask.h"
#include "DEMO_MultiGame.h"
#include "Tasks/TUseItemTask.h"
#include "NiagaraFunctionLibrary.h"


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
	// This behavior leads to creating a task
	// and passing it to the server (gamemode) for processing
	Client_OnItemAcquired();
}


void UInvenComponent::RequestUseItem(const int32 ItemID)
{
	// This behavior leads to creating a task
	// and passing it to the server (gamemode) for processing
	Client_OnItemUsed(ItemID);
}


void UInvenComponent::ProcessItemAcquisition(const FItemData& ItemData)
{
	
#ifdef UE_SERVER
	
	if (OwnerCharacter->HasAuthority())
	{
		Multicast_AcquireItemEffect_Implementation(OwnerCharacter->GetActorLocation());
	}

#endif
	
	ItemList.Add(ItemData);
	TESTLOG(Display, TEXT("Multicast_AddItemToList - Component Address: %p, Item Added: %s, Total Items: %d"), 
		this, *ItemData.ItemName, ItemList.Num());
}


void UInvenComponent::ProcessItemUsage(const int32 ItemID)
{

#ifdef UE_SERVER
	
	if (OwnerCharacter->HasAuthority())
	{
		Multicast_UseItemEffect_Implementation(OwnerCharacter->GetActorLocation());
	}

#endif
	
	// Fine the item in the list and remove it
	int32 Index = -1;
	for (int32 i = 0; i < ItemList.Num(); ++i)
	{
		if (ItemList[i].ItemID == ItemID)
		{
			Index = i;
			break;
		}
	}

	// Remove the item from the list
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


void UInvenComponent::Multicast_AcquireItemEffect_Implementation(const FVector Location)
{
	//#ifndef UE_SERVER
	
	// RPC to display the effect at the targeted player's location
	if (!AcquireItemEffect)
	{
		TESTLOG(Error, TEXT("AcquireItemEffect not set"));
		return;
	}

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), AcquireItemEffect, Location);
	//#endif
}


void UInvenComponent::Multicast_UseItemEffect_Implementation(const FVector Location)
{
	//#ifndef UE_SERVER
	
	// RPC to display the effect at the targeted player's location
	if (!UseItemEffect)
	{
		TESTLOG(Error, TEXT("HitEffect not set"));
		return;
	}

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), UseItemEffect, Location);
	TESTLOG(Warning, TEXT("UseItemEffect: %p"), UseItemEffect);
	//#endif
}



///////////////////////////////////////////////////////////////////////////
///////////               RPC FUNCTIONS                   /////////////////
///////////////////////////////////////////////////////////////////////////



void UInvenComponent::Client_OnItemAcquired_Implementation()
{
	Server_RequestAcquireItem();
}


void UInvenComponent::Client_OnItemUsed_Implementation(int32 ItemID)
{
	// Check if the item is in the client list
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
		// Call the server to process the item usage
		Server_RequestUseItem(ItemID);
	}
	else
	{
		TESTLOG(Warning, TEXT("Client tried to use non-existent item: %d"), ItemID);
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

	
	FTAcquireItemTask* Task = new FTAcquireItemTask();
	if (!Task)
	{
		TESTLOG(Error, TEXT("Failed to get or create attack task"));
		return;
	}

	
	// Create a random new item
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

	// Send the task you created to the server to process equipping items
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

	// Send the task you created to the server to process using items
	GameMode->ExecuteUseItemTask(Task);

#endif
}




