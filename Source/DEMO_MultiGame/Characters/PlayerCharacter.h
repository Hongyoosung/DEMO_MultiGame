// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DEMO_MultiGameCharacter.h"
#include "NiagaraFunctionLibrary.h"
#include "PlayerCharacter.generated.h"

// Forward declarations
class AMultiGameMode;
class UAntiCheatManager;
class UHealthComponent;
class UPlayerUIComponent;
class UAntiCheatComponent;

/**
 * Struct for player security checksums
 */
USTRUCT()
struct FPlayerChecksum
{
    GENERATED_BODY()
public:
    FPlayerChecksum() : HealthChecksum(0), PositionChecksum(0), LastChecksumPosition(FVector3d(0, 0, 0)) {}

    // Getters
    uint32  GetHealthChecksum() const { return HealthChecksum; }
    uint32  GetPositionChecksum() const { return PositionChecksum; }
    FVector GetLastChecksumPosition() const { return LastChecksumPosition; }

    // Setters
    void SetHealthChecksum(const uint32 Checksum) { HealthChecksum = Checksum; }
    void SetPositionChecksum(const uint32 Checksum) { PositionChecksum = Checksum; }
    void SetLastChecksumPosition(const FVector& Position) { LastChecksumPosition = Position; }
    
    // Update methods
    void UpdateHealthChecksum(const float Health);
    void UpdatePositionChecksum(const FVector& Position);

private:
    UPROPERTY()
    uint32 HealthChecksum;

    UPROPERTY()
    uint32 PositionChecksum;

    UPROPERTY()
    FVector LastChecksumPosition;
};

/**
 * Character class for players with health, UI, and anti-cheat functionality
 */
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

    // Take damage function
    UFUNCTION()
    void TakeDamage(float Damage);

    // Checksum access for verification
    const FPlayerChecksum& GetChecksums() const { return Checksums; }

    // Attack range getter
    float GetAttackRange() const { return AttackRange; }

    UHealthComponent* GetHealthComponent() const { return HealthComponent; }
    
protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
    
private:
    // Attack RPCs
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_Attack();
    void Client_Attack();

    // Update all security checksums
    void UpdateAllChecksums();

public:
    // Effects
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Effects")
    UNiagaraSystem* HitEffect;

private:
    // Game references
    UPROPERTY()
    AMultiGameMode* GameMode;

    UPROPERTY()
    UAntiCheatManager* AntiCheatManager;
    
    // Component references - will be created in constructor
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UHealthComponent* HealthComponent;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UPlayerUIComponent* UIComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UAntiCheatComponent* AntiCheatComponent;
    
    
    // Properties
    UPROPERTY()
    float AttackRange;
    
    UPROPERTY()
    FPlayerChecksum Checksums;

    UPROPERTY(EditDefaultsOnly, Category = "AntiCheat", meta = (AllowPrivateAccess = "true"))
    float ChecksumUpdateInterval;

    float TimeSinceLastChecksumUpdate;

    // Getters for components - friend classes can use these
    friend class UHealthComponent;
    friend class UPlayerUIComponent;
    friend class UAntiCheatComponent;
};