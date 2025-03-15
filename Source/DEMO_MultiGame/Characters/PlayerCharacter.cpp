// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCharacter.h"
#include "DEMO_MultiGame.h"
#include "Components/HealthComponent.h"
#include "Components/PlayerUIComponent.h"
#include "Components/AntiCheatComponent.h"
#include "Components/InvenComponent.h"
#include "Managers/AntiCheatManager.h"
#include "GameModes/MultiGameMode.h"
#include "Tables/ItemData.h"
#include "Compression/CompressedBuffer.h"
#include "Kismet/GameplayStatics.h"


APlayerCharacter::APlayerCharacter()
    : GameMode(nullptr)
    , AttackRange(300.0f)
{
    // Create components
    HealthComponent     =   CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
    UIComponent         =   CreateDefaultSubobject<UPlayerUIComponent>(TEXT("UIComponent"));
    AntiCheatComponent  =   CreateDefaultSubobject<UAntiCheatComponent>(TEXT("AntiCheatComponent"));
    InvenComponent      =   CreateDefaultSubobject<UInvenComponent>(TEXT("InvenComponent"));

    
    bReplicates = true;
}


void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    PlayerInputComponent->BindAction("Attack",      IE_Pressed, this, &APlayerCharacter::Attack);
    PlayerInputComponent->BindAction("AcquireItem", IE_Pressed, this, &APlayerCharacter::AcquireItem); 
    PlayerInputComponent->BindAction("UseItem",     IE_Pressed, this, &APlayerCharacter::UseItem); 
}

void APlayerCharacter::InitializeManagers()
{
    // Initialize AntiCheatManager
    AntiCheatManager = GameMode->GetAntiCheatManager();
    if (!AntiCheatManager)
    {
        TESTLOG(Warning, TEXT("Failed to get AntiCheatManager"));
        return;
    }
}


void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    // 서버에서만 GameMode 초기화
    if (HasAuthority()) // 서버에서만 실행
    {
        if (AMultiGameMode* GM = Cast<AMultiGameMode>(UGameplayStatics::GetGameMode(this)))
        {
            GameMode = GM;
            AntiCheatComponent->InitializeGameMode(GM);
            InvenComponent->InitializeGameMode(GM);
        }
        else
        {
            TESTLOG(Error, TEXT("Failed to get GameMode on server"));
        }
    }
}


void APlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    static float AccumulatedTime = 0.0f;
    AccumulatedTime += DeltaTime;

    if (AccumulatedTime >= 0.2f)
    {
        //AntiCheatComponent->UpdateAllChecksums();
        
        AccumulatedTime = 0.0f;
    }
}


void APlayerCharacter::TakeDamage(const float Damage) const
{
#ifdef UE_SERVER
    if (HealthComponent)
    {
        HealthComponent->TakeDamage(Damage);
    }
#endif
}


void APlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}


bool APlayerCharacter::AttackVerification(const APlayerCharacter* Player) const
{
    return GameMode->GetAntiCheatManager()->VerifyAllChecksums(this);
}


bool APlayerCharacter::ItemVerification(const APlayerCharacter* Player, const int32 ItemID) const
{
    return AntiCheatManager->VerifyItemUsage(Player, ItemID);
}

bool APlayerCharacter::PlayerVerification(const APlayerCharacter* Player) const
{
    return AntiCheatManager->VerifyPlayerValid(Player);
}

void APlayerCharacter::Attack() 
{
    HealthComponent->Attack();
}

void APlayerCharacter::UseItem()
{
    TESTLOG(Display, TEXT("UseItem called - InvenComponent Address: %p"), InvenComponent);
    
    TArray<int32> ItemIDs;

    // 아이템 목록에서 사용 가능한 아이템 ID 수집
    for (const auto& Item : InvenComponent->GetItemList())
    {
        ItemIDs.Add(Item.ItemID);
    }

    if (ItemIDs.Num() > 0)
    {
        // 랜덤 아이템 선택
        const int32 ItemID = ItemIDs[FMath::RandRange(0, ItemIDs.Num() - 1)];
        TESTLOG(Display, TEXT("Using item with ID: %d"), ItemID);
        
        // 아이템 사용 함수 호출
        InvenComponent->RequestUseItem(ItemID);
    }
    else
    {
        TESTLOG(Warning, TEXT("No items in inventory to use"));
    }
}


void APlayerCharacter::AcquireItem()
{
    TESTLOG(Display, TEXT("AcquireItem called - InvenComponent Address: %p"), InvenComponent);
    
    InvenComponent->RequestAcquireItem();
}
