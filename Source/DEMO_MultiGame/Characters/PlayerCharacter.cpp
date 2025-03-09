// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacter.h"
#include "DEMO_MultiGame.h"
#include "Net/UnrealNetwork.h"
#include "Components/HealthComponent.h"
#include "Components/PlayerUIComponent.h"
#include "Components/AntiCheatComponent.h"
#include "Managers/AntiCheatManager.h"
#include "GameModes/MultiGameMode.h"

void FPlayerChecksum::UpdateHealthChecksum(const float Health)
{
    HealthChecksum = FCrc::MemCrc32(&Health, sizeof(Health));
}

void FPlayerChecksum::UpdatePositionChecksum(const FVector& Position)
{
    PositionChecksum = FCrc::MemCrc32(&Position, sizeof(Position));
}

APlayerCharacter::APlayerCharacter()
    : HitEffect(nullptr)
    , GameMode(nullptr)
    , AttackRange(300.0f)
    , ChecksumUpdateInterval(1.0f)
    , TimeSinceLastChecksumUpdate(0.0f)
{
    // Create health component
    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
    
    // Create UI component
    UIComponent = CreateDefaultSubobject<UPlayerUIComponent>(TEXT("UIComponent"));
    
    // Create anti-cheat component
    AntiCheatComponent = CreateDefaultSubobject<UAntiCheatComponent>(TEXT("AntiCheatComponent"));
    
    // Initialize checksums
    UpdateAllChecksums();
}

void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    FPlatformProcess::Sleep(0.2f);
    
    // Initialize GameMode
    GameMode = Cast<AMultiGameMode>(GetWorld()->GetAuthGameMode());
    if (!GameMode)
    {
        TESTLOG(Warning, TEXT("Failed to get GameMode"));
    }
    
    // Checksums are initialized in AntiCheatComponent
}

void APlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Delegate checksum updates to the AntiCheatComponent
    
    if (HasAuthority() && AntiCheatComponent)
    {
        AntiCheatComponent->TickChecksumUpdate(DeltaTime);
    }
    
}

void APlayerCharacter::TakeDamage(const float Damage)
{
    if (HasAuthority() && HealthComponent)
    {
        const float NewHealth = FMath::Max(0.0f, HealthComponent->GetHealth() - Damage);
        HealthComponent->SetHealth(NewHealth);
        Multicast_SpawnHitEffect(GetActorLocation());
    }
}

bool APlayerCharacter::Server_Attack_Validate()
{
    if (!GameMode || !GameMode->GetAntiCheatManager())
    {
        TESTLOG(Warning, TEXT("Failed to get GameMode or AntiCheatManager"));
        return false;
    }
    
    return GameMode->GetAntiCheatManager()->VerifyPlayerValid(this);
}

void APlayerCharacter::Server_Attack_Implementation()
{
    if (!GameMode)
    {
        TESTLOG(Warning, TEXT("Failed to get GameMode"));
        return;
    }

    AntiCheatManager = GameMode->GetAntiCheatManager();
    if (!AntiCheatManager)
    {
        TESTLOG(Warning, TEXT("Failed to get AntiCheatManager"));
        return;
    }
    
    // Verify checksums
    if (!AntiCheatManager->VerifyAllChecksums(this) ||
        !AntiCheatManager->VerifyPositionChecksum(this))
    {
        TESTLOG(Warning, TEXT("Checksum verification failed for player %s"), *GetName());
        return;
    }
    
    // Register attack task in thread pool
    GameMode->ExecuteAttackTask(this);
}

void APlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    // Health is now replicated in HealthComponent
}

void APlayerCharacter::Multicast_SpawnHitEffect_Implementation(const FVector Location)
{
    // RPC to display the effect at the targeted player's location
    if (!HitEffect)
    {
        TESTLOG(Error, TEXT("HitEffect not set"));
        return;
    }

    UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitEffect, Location);
}

void APlayerCharacter::UpdateAllChecksums()
{
    Checksums.SetLastChecksumPosition(GetActorLocation());
    if (HealthComponent)
    {
        Checksums.UpdateHealthChecksum(HealthComponent->GetHealth());
    }
    Checksums.UpdatePositionChecksum(Checksums.GetLastChecksumPosition());
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &APlayerCharacter::Client_Attack);
}

void APlayerCharacter::Client_Attack()
{
    Server_Attack();
}