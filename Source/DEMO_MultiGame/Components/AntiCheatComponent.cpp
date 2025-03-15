#include "AntiCheatComponent.h"
#include "DEMO_MultiGame.h"
#include "HealthComponent.h"
#include "Characters/PlayerCharacter.h"
#include "GameModes/MultiGameMode.h"
#include "Managers/AntiCheatManager.h"


UAntiCheatComponent::UAntiCheatComponent()
    : ChecksumUpdateInterval(1.0f)
    , TimeSinceLastChecksumUpdate(0.0f)
    , OwnerCharacter(nullptr), GameMode(nullptr)
{
    PrimaryComponentTick.bCanEverTick = false;
}


void UAntiCheatComponent::UpdateAllChecksums()
{
    const float     Health              = OwnerCharacter->GetHealthComponent()->GetHealth();
    const int32     HealthChecksum      = FCrc::MemCrc32(&Health, sizeof(Health));

    const FVector   Position            = OwnerCharacter->GetActorLocation();
    const int32     PositionChecksum    = FCrc::MemCrc32(&Position, sizeof(FVector));
    
    Checksums.SetHealthChecksum         (HealthChecksum);
    Checksums.SetPositionChecksum       (PositionChecksum);
    Checksums.SetLastChecksumPosition   (Position);
}


void UAntiCheatComponent::BeginPlay()
{
    Super::BeginPlay();
    
    OwnerCharacter = Cast<APlayerCharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        TESTLOG(Error, TEXT("AntiCheatComponent not attached to PlayerCharacter"));
        return;
    }
}


void UAntiCheatComponent::InitializeGameMode(AMultiGameMode* InGameMode)
{
    GameMode = InGameMode;
    
    if (!GameMode)
    {
        TESTLOG(Error, TEXT("Failed to get MultiGameMode"));
        return;
    }
}


bool UAntiCheatComponent::ValidatePlayerForAction() const
{
    if (!OwnerCharacter)
    {
        TESTLOG(Error, TEXT("AntiCheatComponent not attached to PlayerCharacter"));
        return false;
    }
    
    // Acquire the Anti-Cheat Manager
    UAntiCheatManager* AntiCheatManager = GetAntiCheatManager();
    if (!AntiCheatManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to get AntiCheatManager"));
        return false;
    }
    
    // Delegating player validation to a manager
    return AntiCheatManager->VerifyPlayerValid(OwnerCharacter);
}


UAntiCheatManager* UAntiCheatComponent::GetAntiCheatManager() const
{
    if (!OwnerCharacter || !OwnerCharacter->GetWorld())
    {
        return nullptr;
    }
    
    if (!GameMode)
    {
        return nullptr;
    }
    
    return GameMode->GetAntiCheatManager();
}


bool UAntiCheatComponent::IsTargetInRange(const APlayerCharacter* Target) const
{
    if (!OwnerCharacter || !Target)
    {
        return false;
    }

    
    const UAntiCheatManager* AntiCheatManager = GetAntiCheatManager();
    if (!AntiCheatManager)
    {
        return false;
    }

    
    // Delegate attack scope validation to a manager
    return AntiCheatManager->VerifyAttackRange(
        OwnerCharacter, 
        Target, 
        OwnerCharacter->GetAttackRange()
    );
}
