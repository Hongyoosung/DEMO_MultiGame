// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DEMO_MultiGameCharacter.h"
#include "NiagaraFunctionLibrary.h"
#include "PlayerCharacter.generated.h"

class UProgressBar;
/**
 * 
 */
UCLASS()
class DEMO_MULTIGAME_API APlayerCharacter : public ADEMO_MultiGameCharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

private:
	void Client_Attack();

	UFUNCTION(Server, Reliable)
	void Server_Attack();

	UFUNCTION()
	void TakeDamage(float Damage);

public:
	UPROPERTY(Replicated, BlueprintReadOnly)
	float Health;

	// niagara system
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class UNiagaraSystem* HitEffect;
	
private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> HealthWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	UUserWidget* HealthWidget;

	UPROPERTY(EditDefaultsOnly)
	UProgressBar* HealthBar;
	
};
