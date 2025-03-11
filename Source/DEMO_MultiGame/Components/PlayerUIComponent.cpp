// Implementation of the Player UI Component

#include "PlayerUIComponent.h"
#include "Characters/PlayerCharacter.h"
#include "Widgets/HealthBarWidget.h"
#include "Components/HealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "DEMO_MultiGame.h"

UPlayerUIComponent::UPlayerUIComponent()
    : HealthBarWidgetComponent(nullptr)
    , HealthBarWidget(nullptr)
    , HealthWidgetClass(nullptr)
    , OwnerCharacter(nullptr)
{
    // Set this component to be initialized when the game starts, and to be ticked every frame
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UPlayerUIComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Get owner reference
    OwnerCharacter = Cast<APlayerCharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        TESTLOG(Error, TEXT("PlayerUIComponent not attached to PlayerCharacter"));
        return;
    }

//#ifndef UE_SERVER
    // Create widget component if it doesn't exist
    if (!HealthBarWidgetComponent)
    {
        HealthBarWidgetComponent = NewObject<UWidgetComponent>(OwnerCharacter, UWidgetComponent::StaticClass(), TEXT("HealthBar"));
        HealthBarWidgetComponent->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform);
        HealthBarWidgetComponent->SetRelativeScale3D(FVector(0.1f, 0.1f, 0.1f));
        HealthBarWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f));
        HealthBarWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
        HealthBarWidgetComponent->SetDrawSize(FVector2D(200.0f, 20.0f));
        HealthBarWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        HealthBarWidgetComponent->SetGenerateOverlapEvents(false);
        HealthBarWidgetComponent->SetIsReplicated(true);
        
        OwnerCharacter->AddInstanceComponent(HealthBarWidgetComponent);
        HealthBarWidgetComponent->RegisterComponent();
    }
    
    // Initialize UI
    InitializeHealthWidget();
    
    // Subscribe to health changes
    if (UHealthComponent* HealthComp = OwnerCharacter->FindComponentByClass<UHealthComponent>())
    {
        HealthComp->OnHealthChanged.AddDynamic(this, &UPlayerUIComponent::UpdateHealthUI);
    }
//#endif
}

void UPlayerUIComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    // Add any replicated properties here
}

void UPlayerUIComponent::InitializeHealthWidget()
{
//#ifndef UE_SERVER
    if (!HealthBarWidgetComponent)
    {
        TESTLOG(Error, TEXT("HealthBarWidgetComponent not set"));
        return;
    }
    
    if (!HealthWidgetClass)
    {
        TESTLOG(Error, TEXT("HealthWidgetClass not set"));
        return;
    }
    
    HealthBarWidgetComponent->SetWidgetClass(HealthWidgetClass);
    HealthBarWidgetComponent->InitWidget();
    HealthBarWidget = Cast<UHealthBarWidget>(HealthBarWidgetComponent->GetUserWidgetObject());
    
    if (!HealthBarWidget)
    {
        TESTLOG(Warning, TEXT("Failed to create HealthBarWidget"));
        return;
    }
    
    // Get initial health value
    if (OwnerCharacter)
    {
        if (UHealthComponent* HealthComp = OwnerCharacter->FindComponentByClass<UHealthComponent>())
        {
            UpdateHealthUI(HealthComp->GetHealth() / 100.0f);
        }
    }
//#endif
}

void UPlayerUIComponent::UpdateHealthUI(const float HealthPercent)
{
//#ifndef UE_SERVER
    TESTLOG(Warning, TEXT("UpdateHealthUI: %f"), HealthPercent);
    if (!HealthBarWidget)
    {
        return;
    }
    
    // Update Health Percent
    HealthBarWidget->UpdateHealthBar(HealthPercent);
    
    // Update Health Bar Color based on local control
    bool bIsLocallyControlled = false;
    if (OwnerCharacter)
    {
        bIsLocallyControlled = OwnerCharacter->IsLocallyControlled();
    }
    HealthBarWidget->UpdateHealthBarColor(bIsLocallyControlled);
//#endif
}