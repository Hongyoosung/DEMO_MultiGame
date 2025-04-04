﻿// Implementation of the Health Component

#include "HealthComponent.h"
#include "AntiCheatComponent.h"
#include "DEMO_MultiGame.h"
#include "Characters/PlayerCharacter.h"
#include "GameModes/MultiGameMode.h"
#include "Net/UnrealNetwork.h"
#include "Tasks/TAttackTask.h"
#include "NiagaraFunctionLibrary.h"


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


void UHealthComponent::Attack()
{
	// Execute an attack function on the client,
	// which leads to the delivery of an attack task to the server
	Client_Attack();
}


void UHealthComponent::TakeDamage(const float Damage)
{
	
#ifdef UE_SERVER
	
	if (OwnerCharacter->HasAuthority())
	{
		const float NewHealth = FMath::Max(0.0f, GetHealth() - Damage);
		SetHealth(NewHealth);
		Multicast_SpawnHitEffect(OwnerCharacter->GetActorLocation());
	}
	
#endif
}


void UHealthComponent::Multicast_SpawnHitEffect_Implementation(const FVector Location)
{
//#ifndef UE_SERVER
	
	// RPC to display the effect at the targeted player's location
	if (!HitEffect)
	{
		TESTLOG(Error, TEXT("HitEffect not set"));
		return;
	}

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitEffect, Location);
//#endif
}


void UHealthComponent::SetHealth(const float NewHealth)
{
	const float OldHealth = Health;
	Health = FMath::Clamp(NewHealth, 0.0f, MaxHealth);

#ifdef UE_SERVER
	// Update checksums on server
	if (GetOwnerRole() == ROLE_Authority && OwnerCharacter)
	{
		OwnerCharacter->GetAntiCheatComponent()->UpdateAllChecksums();
	}
#endif
}


void UHealthComponent::OnRep_Health()
{
//#ifndef UE_SERVER
	HealthPercent = (MaxHealth > 0.0f) ? (Health / MaxHealth) : 0.0f;
	
	OnHealthChanged.Broadcast(HealthPercent); // Notify listeners about health change
//#endif
}



///////////////////////////////////////////////////////////////////////////
///////////               RPC FUNCTIONS                   /////////////////
///////////////////////////////////////////////////////////////////////////



void UHealthComponent::Client_Attack()
{
	Server_Attack();
}


bool UHealthComponent::Server_Attack_Validate()
{
#ifdef UE_SERVER

	if (!OwnerCharacter)
	{
		TESTLOG(Warning, TEXT("Invalid ThreadPool or Player"));
		return false;
	}
	
	return	OwnerCharacter->PlayerVerification(OwnerCharacter);
			//OwnerCharacter->AttackVerification(OwnerCharacter);
	
#endif
}


void UHealthComponent::Server_Attack_Implementation()
{
#ifdef UE_SERVER

	// Create a task and set an owner
	FTAttackTask* Task = OwnerCharacter->GetGameMode()->GetOrCreateAttackTask();
	if (!Task)
	{
		TESTLOG(Error, TEXT("Failed to get or create attack task"));
		return;
	}

	Task->InitializePlayerValues(OwnerCharacter);

	// Passing attack tasks to the server (game mode)
	OwnerCharacter->GetGameMode()->ExecuteAttackTask(Task);
	
#endif
}