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

	
	void Attack();
	void TakeDamage(const float Damage);

	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	
	// Health getters and setters
	float	GetHealth()	const	{	return Health;	}
	void	SetHealth(const float NewHealth);

	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Attack();
	void Client_Attack();
	
	// Health change notification
	UFUNCTION()
	void OnRep_Health();

	// Delegate for health changes
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChangedSignature, float, NewHealth);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChangedSignature OnHealthChanged;


protected:
	virtual void BeginPlay() override;
	virtual void InitializeGameState();

private:
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_SpawnHitEffect(const FVector Location);

	
public:
	// Effects
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effects")
	UNiagaraSystem* HitEffect;


private:
	UPROPERTY(ReplicatedUsing = OnRep_Health)
	float Health;

	UPROPERTY(EditAnywhere, Category = "Health")
	float MaxHealth;

	float HealthPercent;
    
	UPROPERTY()
	APlayerCharacter* OwnerCharacter;
};