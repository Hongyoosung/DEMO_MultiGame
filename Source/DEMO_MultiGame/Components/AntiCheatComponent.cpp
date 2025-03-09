// AntiCheatComponent.cpp 구현
#include "AntiCheatComponent.h"
#include "Characters/PlayerCharacter.h"
#include "GameModes/MultiGameMode.h"
#include "Managers/AntiCheatManager.h"

UAntiCheatComponent::UAntiCheatComponent()
    : ChecksumUpdateInterval(1.0f)
    , TimeSinceLastChecksumUpdate(0.0f)
    , OwnerCharacter(nullptr)
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UAntiCheatComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // 소유자 참조 가져오기
    OwnerCharacter = Cast<APlayerCharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("AntiCheatComponent not attached to PlayerCharacter"));
        return;
    }
    
    // 초기 체크섬 업데이트
    UpdateAllChecksums();
}

void UAntiCheatComponent::UpdateAllChecksums()
{
    if (!OwnerCharacter)
    {
        return;
    }
    
    // PlayerCharacter의 체크섬 업데이트 메서드 호출
    OwnerCharacter->UpdateAllChecksums();
}

bool UAntiCheatComponent::ValidatePlayerForAction() const
{
    if (!OwnerCharacter)
    {
        return false;
    }
    
    // 안티 치트 매니저 획득
    UAntiCheatManager* AntiCheatManager = GetAntiCheatManager();
    if (!AntiCheatManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to get AntiCheatManager"));
        return false;
    }
    
    // 플레이어 유효성 검증을 매니저에 위임
    return AntiCheatManager->VerifyPlayerValid(OwnerCharacter);
}

UAntiCheatManager* UAntiCheatComponent::GetAntiCheatManager() const
{
    if (!OwnerCharacter || !OwnerCharacter->GetWorld())
    {
        return nullptr;
    }
    
    AMultiGameMode* GameMode = Cast<AMultiGameMode>(OwnerCharacter->GetWorld()->GetAuthGameMode());
    if (!GameMode)
    {
        return nullptr;
    }
    
    return GameMode->GetAntiCheatManager();
}

void UAntiCheatComponent::TickChecksumUpdate(float DeltaTime)
{
    TimeSinceLastChecksumUpdate += DeltaTime;
    if (TimeSinceLastChecksumUpdate >= ChecksumUpdateInterval)
    {
        UpdateAllChecksums();
        TimeSinceLastChecksumUpdate = 0.0f;
    }
}

bool UAntiCheatComponent::IsTargetInRange(const APlayerCharacter* Target) const
{
    if (!OwnerCharacter || !Target)
    {
        return false;
    }
    
    UAntiCheatManager* AntiCheatManager = GetAntiCheatManager();
    if (!AntiCheatManager)
    {
        return false;
    }
    
    // 공격 범위 검증을 매니저에 위임
    return AntiCheatManager->VerifyAttackRange(
        OwnerCharacter, 
        Target, 
        OwnerCharacter->GetAttackRange()
    );
}

// AntiCheatManager.cpp - 기존 코드 유지