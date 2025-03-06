// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DEMO_MultiGameCharacter.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/WidgetComponent.h"
#include "PlayerCharacter.generated.h"



class UHealthBarWidget;
class UAntiCheatManager;


UCLASS()
class DEMO_MULTIGAME_API APlayerCharacter : public ADEMO_MultiGameCharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();

	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	
	// Spawn Hit Effect
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_SpawnHitEffect(const FVector Location);

	
	UFUNCTION()
	void TakeDamage(float Damage);

	
	UFUNCTION(Category = "UI")
	void OnRep_Health() const;

	
	// Health Setter & Getter
	void SetHealth(const float NewHealth)		{	Health = NewHealth;	}
	float GetHealth() const				{	return	 Health;	}

	
protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	
private:
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Attack();
	void Client_Attack();

	
	// UI 관련 함수들
	void InitializeHealthWidget();
	void UpdateHealthUI() const;

	
public:
	// niagara system
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UNiagaraSystem* HitEffect;

	
	// UI
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> HealthWidgetClass;

	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	UWidgetComponent* HealthBarWidgetComponent;

	
private:
	UPROPERTY()
	UHealthBarWidget* HealthBarWidget;

	
	UPROPERTY(ReplicatedUsing = OnRep_Health)
	float Health;


	friend class UAntiCheatManager;
	uint32 HealthChecksum;
	
};
