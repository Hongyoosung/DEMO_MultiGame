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


TArray<FItemData> APlayerCharacter::GetItemList() const
{
    return InvenComponent->GetItemList();
}

void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize gamemode on server only
    if (HasAuthority())
    {
        if (AMultiGameMode* GM = Cast<AMultiGameMode>(UGameplayStatics::GetGameMode(this)))
        {
            GameMode = GM;
            
            AntiCheatComponent  ->  InitializeGameMode(GM);
            InvenComponent      ->  InitializeGameMode(GM);
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


void APlayerCharacter::TakeAcquireItem(const FItemData& Item) const
{
    
#ifdef UE_SERVER
    
    if (InvenComponent)
    {
        InvenComponent->ProcessItemAcquisition(Item);
    }
    
#endif
    
}


void APlayerCharacter::TakeUseItem(const int32 ItemID) const
{
    
#ifdef UE_SERVER
    
    if (InvenComponent)
    {
        InvenComponent->ProcessItemUsage(ItemID);
    }
    
#endif
    
}


void APlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}


bool APlayerCharacter::AttackVerification(const APlayerCharacter* Player) const
{
    return GameMode->GetAntiCheatManager()->VerifyPositionChecksum(this);
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
    
    // Collecting available item IDs from an item list 
    for (const auto& Item : InvenComponent->GetItemList())
    {
        ItemIDs.Add(Item.ItemID);
    }

    
    if (ItemIDs.Num() > 0)
    {
        // Selection Random Item ID
        const int32 ItemID = ItemIDs[FMath::RandRange(0, ItemIDs.Num() - 1)];
        TESTLOG(Display, TEXT("Using item with ID: %d"), ItemID);

        // Call using item function
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
