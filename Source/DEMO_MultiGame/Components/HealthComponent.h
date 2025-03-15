// Health component to manage player health

#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

class APlayerCharacter;
class FTAttackTask;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DEMO_MULTIGAME_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	
	// Attack Action interface function
	void Attack();
	void TakeDamage(const float Damage);

	
	// Health change notification
	UFUNCTION()
	void OnRep_Health();

	
	// Health getter and setters
	void SetHealth(const float NewHealth);
	FORCEINLINE	float GetHealth() const	{	return Health;	}

	
	// Delegate for health changes
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChangedSignature, float, NewHealth);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChangedSignature OnHealthChanged;

	
protected:
	virtual void BeginPlay() override;
	

private:
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Attack();
	void Client_Attack();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_SpawnHitEffect(const FVector Location);

	
public:
	// Effects
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effects")
	UNiagaraSystem* HitEffect;


private:
	UPROPERTY()
	APlayerCharacter* OwnerCharacter;
	
	UPROPERTY(ReplicatedUsing = OnRep_Health)
	float Health;

	UPROPERTY(EditAnywhere, Category = "Health")
	float MaxHealth;

	float HealthPercent;
};