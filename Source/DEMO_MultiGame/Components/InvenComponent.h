#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Tables/ItemData.h"
#include "InvenComponent.generated.h"


class AMultiGameMode;
class APlayerCharacter;


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DEMO_MULTIGAME_API UInvenComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInvenComponent();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void InitializeGameMode(AMultiGameMode* InGameMode);

	
	void RequestAcquireItem();
	void RequestUseItem(const int32 ItemID);

	
	UFUNCTION(Client, Reliable)
	void Client_OnItemAcquired();
	UFUNCTION(Client, Reliable)
	void Client_OnItemUsed(int32 ItemID);


	void ProcessItemAcquisition(const FItemData& ItemData);
	void ProcessItemUsage(int32 ItemID);


	FORCEINLINE const	TArray<FItemData>& GetItemList() const	{ return ItemList; }
	FORCEINLINE			TArray<FItemData>& GetItemList()		{ return ItemList; }

	
private:
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestAcquireItem();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestUseItem(int32 ItemID);

	
private:
	UPROPERTY(Replicated)
	TArray<FItemData> ItemList;

	UPROPERTY()
	AMultiGameMode* GameMode;

	UPROPERTY()
	APlayerCharacter* OwnerCharacter;
};
