#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InvenComponent.generated.h"

struct FItemData;
class AMultiGameMode;
class APlayerCharacter;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DEMO_MULTIGAME_API UInvenComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInvenComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	TArray<FItemData>& GetItemDataList() { return ItemList; }

	void AcquireItem();
	void UseItem(const int32 ItemID);

protected:
	virtual void BeginPlay() override;

	
private:
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_AcquireItem();
	void Client_AcquireItem();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_UseItem(int32 ItemID);
	void Client_UseItem(int32 ItemID);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_AddItemToList(FItemData NewItem);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_RemoveItemFromList(int32 RemovedItemID);

	
private:
	UPROPERTY()
	TArray<FItemData> ItemList;

	UPROPERTY()
	AMultiGameMode* GameMode;

	UPROPERTY()
	APlayerCharacter* OwnerCharacter;
};
