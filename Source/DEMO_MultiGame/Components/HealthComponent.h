// Health component to manage player health

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

class APlayerCharacter;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DEMO_MULTIGAME_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Health getters and setters
	float	GetHealth()					const	{	return Health;	}
	void	SetHealth(float NewHealth);
    
	// Health change notification
	UFUNCTION()
	void OnRep_Health();

	// Delegate for health changes
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChangedSignature, float, NewHealth);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChangedSignature OnHealthChanged;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(ReplicatedUsing = OnRep_Health)
	float Health;

	UPROPERTY(EditAnywhere, Category = "Health")
	float MaxHealth;

	float HealthPercent;
    
	UPROPERTY()
	APlayerCharacter* OwnerCharacter;
};