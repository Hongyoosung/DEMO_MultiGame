// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Kismet/GameplayStatics.h"


APlayerCharacter::APlayerCharacter() : Health(100.0f)
{
	
}

void APlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APlayerCharacter, Health);
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HealthWidgetClass && IsLocallyControlled())
	{
		HealthWidget = CreateWidget<UUserWidget>(GetWorld(), HealthWidgetClass);
		if (HealthWidget)
		{
			HealthWidget->AddToViewport();
		}
	}
}

void APlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HealthWidget)
	{
		HealthBar = Cast<UProgressBar>(HealthWidget->GetWidgetFromName("HealthBar"));
		if (HealthBar)
		{
			HealthBar->SetPercent(Health / 100.0f);
		}
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &APlayerCharacter::Client_Attack);
}

void APlayerCharacter::Client_Attack()
{
	if (HasAuthority())
	{
		Server_Attack();
	}
	else
	{
		Server_Attack();
	}
}

void APlayerCharacter::TakeDamage(float Damage)
{
	if (HasAuthority())
	{
		Health -= Damage;
		if (Health < 0) Health = 0;

		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitEffect, GetActorLocation());
	}
}

void APlayerCharacter::Server_Attack_Implementation()
{
	TArray<FHitResult> HitResults;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FVector Start = GetActorLocation();
	FVector End = Start;

	bool bHit = GetWorld()->SweepMultiByChannel(HitResults, Start, End, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(300.0f), Params);

	if (bHit)
	{
		for (auto& Hit : HitResults)
		{
			APlayerCharacter* OtherCharacter = Cast<APlayerCharacter>(Hit.GetActor());
			
			if (OtherCharacter && OtherCharacter != this)
			{
				OtherCharacter->TakeDamage(10.0f);
			}
		}
	}
}

