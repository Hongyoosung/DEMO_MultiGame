// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DEMO_MultiGameCharacter.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/WidgetComponent.h"
#include "PlayerCharacter.generated.h"


class AMultiGameMode;
class UHealthBarWidget;
class UAntiCheatManager;

/*
 using CSV (Comma Separated Values) format
USTRUCT()
struct FPlayerData
{
	GENERATED_BODY()

	UPROPERTY()
	FString Name;

	UPROPERTY()
	int32 Level;

	UPROPERTY()
	float Health;

	UPROPERTY()
	FVector Position;
};
*/

USTRUCT()
struct FPlayerChecksum
{
	GENERATED_BODY()
public:
	FPlayerChecksum() : HealthChecksum(0), PositionChecksum(0), LastChecksumPosition(FVector3d(0, 0, 0)) {}

	
	// Getters
	uint32  GetHealthChecksum			()	const						{		return				 HealthChecksum;		}
	uint32  GetPositionChecksum			()	const						{		return				 PositionChecksum;		}
	FVector GetLastChecksumPosition		()	const						{		return				 LastChecksumPosition;	}

	
	// Setters
	void SetHealthChecksum				(const uint32 Checksum)			{		HealthChecksum		 = Checksum;			}
	void SetPositionChecksum			(const uint32 Checksum)			{		PositionChecksum	 = Checksum;			}
	void SetLastChecksumPosition		(const FVector& Position)		{		LastChecksumPosition = Position;			}
	
	// Update methods
	void UpdateHealthChecksum			(const float Health	);
	void UpdatePositionChecksum			(const FVector& Position);

	
private:
	UPROPERTY()
	uint32 HealthChecksum;

	UPROPERTY()
	uint32 PositionChecksum;

	UPROPERTY()
	FVector LastChecksumPosition;
};



UCLASS()
class DEMO_MULTIGAME_API APlayerCharacter : public ADEMO_MultiGameCharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;

	
	// Spawn Hit Effect
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_SpawnHitEffect(const FVector Location);

	
	UFUNCTION()
	void TakeDamage(float Damage);
	UFUNCTION(Category = "UI")
	void OnRep_Health() const;

	
	// Health Setter & Getter
	void	SetHealth		(const float NewHealth);
	void	SetAttackRange	(const float NewRange)		{		AttackRange =	NewRange;		}
	float	GetHealth		() const					{		return			Health;			}
	float	GetAttackRange	() const					{		return			AttackRange;	}

	
	// Checksum access for verification
	const	FPlayerChecksum& GetChecksums() const		{		return		Checksums;	}

	
protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	
private:
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Attack();
	void Client_Attack();

	
	// UI related functions
	void InitializeHealthWidget	();
	void UpdateHealthUI			()	const;

	
	// Update all security checksums
	void UpdateAllChecksums();

	
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
	AMultiGameMode* GameMode;
	
	UPROPERTY()
	UHealthBarWidget* HealthBarWidget;
	
	UPROPERTY(ReplicatedUsing = OnRep_Health)
	float Health;

	UPROPERTY()
	float AttackRange;
	
	UPROPERTY()
	FPlayerChecksum Checksums;

	UPROPERTY(EditDefaultsOnly, Category = "AntiCheat", meta = (AllowPrivateAccess = "true"))
	float ChecksumUpdateInterval;

	float TimeSinceLastChecksumUpdate;
};
