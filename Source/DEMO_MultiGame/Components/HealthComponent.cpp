// Implementation of the Health Component

#include "HealthComponent.h"
#include "Characters/PlayerCharacter.h"
#include "Net/UnrealNetwork.h"

UHealthComponent::UHealthComponent()
	: Health(100.0f)
	, MaxHealth(100.0f)
	, OwnerCharacter(nullptr)
	, HealthPercent(100.0f)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	SetHealth(MaxHealth);
    
	// Get owner reference
	OwnerCharacter = Cast<APlayerCharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("HealthComponent not attached to PlayerCharacter"));
	}
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UHealthComponent, Health);
}

void UHealthComponent::SetHealth(float NewHealth)
{
	const float OldHealth = Health;
	Health = FMath::Clamp(NewHealth, 0.0f, MaxHealth);

#ifdef UE_SERVER
	// Update checksums on server
	if (GetOwnerRole() == ROLE_Authority && OwnerCharacter)
	{
		OwnerCharacter->UpdateAllChecksums();
	}
#endif
}

void UHealthComponent::OnRep_Health()
{

//#ifndef UE_SERVER
	HealthPercent = (MaxHealth > 0.0f) ? (Health / MaxHealth) : 0.0f;
	
	// Notify listeners about health change
	OnHealthChanged.Broadcast(HealthPercent);
//#endif
}