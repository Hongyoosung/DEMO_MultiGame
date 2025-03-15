#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Tables/ItemData.h"
#include "NiagaraSystem.h"
#include "InvenComponent.generated.h"


class AMultiGameMode;
class APlayerCharacter;
class UNiagaraSystem;


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DEMO_MULTIGAME_API UInvenComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInvenComponent();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void InitializeGameMode		(AMultiGameMode* InGameMode);

	
	// Item Action Interface functions
	void RequestAcquireItem		();
	void RequestUseItem			(const int32 ItemID);
	

	// Client synchronization functions
	void ProcessItemAcquisition	(const FItemData& ItemData);
	void ProcessItemUsage		(const int32 ItemID);

	
	// Item list getter
	FORCEINLINE const	TArray<FItemData>& GetItemList() const	{ return ItemList; }
	FORCEINLINE			TArray<FItemData>& GetItemList()		{ return ItemList; }

	
private:
	UFUNCTION(Client, Reliable)
	void Client_OnItemAcquired();
	UFUNCTION(Client, Reliable)
	void Client_OnItemUsed(int32 ItemID);

	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestAcquireItem();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestUseItem(int32 ItemID);


	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_AcquireItemEffect(const FVector Location);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_UseItemEffect(const FVector Location);
	
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effects")
	UNiagaraSystem* AcquireItemEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effects")
	UNiagaraSystem* UseItemEffect;
	
	
private:
	UPROPERTY(Replicated)
	TArray<FItemData> ItemList;

	UPROPERTY()
	AMultiGameMode* GameMode;

	UPROPERTY()
	APlayerCharacter* OwnerCharacter;
};
