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




APlayerCharacter::APlayerCharacter()
    : GameMode(nullptr)
    , AttackRange(300.0f)
{

    // Create components
    HealthComponent     =   CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
    UIComponent         =   CreateDefaultSubobject<UPlayerUIComponent>(TEXT("UIComponent"));
    AntiCheatComponent  =   CreateDefaultSubobject<UAntiCheatComponent>(TEXT("AntiCheatComponent"));
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


bool APlayerCharacter::AttackVerification(const APlayerCharacter* Player) const
{
    return GameMode->GetAntiCheatManager()->VerifyAllChecksums(Player);
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
        return;
    }

    FPlatformProcess::Sleep(0.2f);

    if (!GameMode->HasActorBegunPlay())
    {
        TESTLOG(Warning, TEXT("GameMode has not begun play yet. Delaying AntiCheatManager initialization."));
        GetWorld()->GetTimerManager().SetTimerForNextTick(this, &APlayerCharacter::InitializeManagers);
        return;
    }
}


void APlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    static float AccumulatedTime = 0.0f;
    AccumulatedTime += DeltaTime;

    if (AccumulatedTime >= 0.2f)
    {
        AntiCheatComponent->UpdateAllChecksums();
        
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
    TArray<int32> ItemIDs;

    for (auto& Item : InvenComponent->GetItemDataList())
    {
        ItemIDs.Add(Item.ItemID);
    }

    const int32 ItemID = ItemIDs[FMath::RandRange(0, ItemIDs.Num() - 1)];

    InvenComponent->UseItem(ItemID);
}


void APlayerCharacter::AcquireItem()
{
    InvenComponent->AcquireItem();
}
