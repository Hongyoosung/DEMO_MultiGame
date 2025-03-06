// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DEMO_MultiGameCharacter.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/WidgetComponent.h"
#include "PlayerCharacter.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(PlayerCharacter, Log, All);

// 로그를 출력한 함수 이름과 코드라인 번호 출력
#define TESTLOG_CALLINFO (FString(__FUNCTION__) + TEXT("(") + FString::FromInt(__LINE__) + TEXT(")"))
#define TESTLOG_S(Verbosity) UE_LOG(PlayerCharacter, Verbosity, TEXT("%s"), *TESTLOG_CALLINFO)
#define TESTLOG(Verbosity, Format, ...) UE_LOG(PlayerCharacter, Verbosity, TEXT("%s %s"), *TESTLOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))


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
	UPROPERTY(EditDefaultsOnly)
	UHealthBarWidget* HealthBarWidget;

	
	UPROPERTY(ReplicatedUsing = OnRep_Health)
	float Health;


	friend class UAntiCheatManager;
	uint32 HealthChecksum;
	
};
