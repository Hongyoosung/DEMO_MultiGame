// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "DEMO_MultiGame.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/HealthBarWidget.h"
#include "AntiCheat/AntiCheatManager.h"
#include "Modules/ModuleManager.h"
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
	: HitEffect(nullptr), HealthWidgetClass(nullptr)
	, HealthBarWidgetComponent(nullptr), GameMode(nullptr)
	, HealthBarWidget(nullptr), Health(100.0f)
	, AttackRange(300.0f), ChecksumUpdateInterval(1.0f), TimeSinceLastChecksumUpdate(0.0f)
	
{
	
	// Initialize HealthBarWidgetComponent
	HealthBarWidgetComponent =		CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	HealthBarWidgetComponent->		SetupAttachment(GetMesh());
	HealthBarWidgetComponent->		SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f));
	HealthBarWidgetComponent->		SetWidgetSpace(EWidgetSpace::Screen); 
	HealthBarWidgetComponent->		SetDrawSize(FVector2D(200.0f, 50.0f));
	HealthBarWidgetComponent->		SetNetAddressable();
	HealthBarWidgetComponent->		SetIsReplicated(true);

	// Initialize checksums
	UpdateAllChecksums();
}


void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Initialize GameMode
	GameMode = Cast<AMultiGameMode>(GetWorld()->GetAuthGameMode());
	if (!GameMode)
	{
		TESTLOG(Warning, TEXT("Failed to get GameMode"));
	}
	
	InitializeHealthWidget();
	
	// Initialize checksums
	UpdateAllChecksums();
}


void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    
	// Only update checksums periodically to save performance
	if (HasAuthority())
	{
		TimeSinceLastChecksumUpdate += DeltaTime;
		if (TimeSinceLastChecksumUpdate >= ChecksumUpdateInterval)
		{
			UpdateAllChecksums();
			TimeSinceLastChecksumUpdate = 0.0f;
		}
	}
}


void APlayerCharacter::SetHealth(const float NewHealth)
{
	Health = NewHealth;

	// if Server, Update checksyms when health is changed
	if (HasAuthority())
	{
		Checksums.UpdateHealthChecksum(Health);
	}
}


void APlayerCharacter::UpdateAllChecksums()
{
	Checksums.SetLastChecksumPosition(GetActorLocation());
	Checksums.UpdateHealthChecksum(Health);
	Checksums.UpdatePositionChecksum(Checksums.GetLastChecksumPosition());
}


void APlayerCharacter::TakeDamage(const float Damage)
{
	if (HasAuthority() == true)
	{
		if (GetHealth() - Damage< 0)
		{
			SetHealth(0);
		}
		else
		{
			SetHealth(GetHealth() - Damage);
		}

		// Each client displays the effect
		Multicast_SpawnHitEffect(GetActorLocation());
	}
}


bool APlayerCharacter::Server_Attack_Validate()
{
	if (!GameMode)
	{
		TESTLOG(Warning, TEXT("Failed to GameMode"));
		return false;
	}
    
	const UAntiCheatManager* AntiCheatManager = GameMode->GetAntiCheatManager();
	if (!AntiCheatManager)
	{
		return false;
	}
    
	// Check if player is valid
	return AntiCheatManager->VerifyPlayerValid(this);
}


void APlayerCharacter::Server_Attack_Implementation()
{
	if (!GameMode)
	{
		TESTLOG(Warning, TEXT("Failed to get GameMode"));
		return;
	}

	UAntiCheatManager* AntiCheatManager = GameMode->GetAntiCheatManager();
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
	DOREPLIFETIME(APlayerCharacter, Health);
}


void APlayerCharacter::Multicast_SpawnHitEffect_Implementation(const FVector Location)
{
	// 피격 플레이어에 이펙트를 나타내는 RPC
	if (HitEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitEffect, Location);
	}
}


void APlayerCharacter::InitializeHealthWidget()
{
	if (HealthBarWidgetComponent)
	{
		// 위젯 생성 및 초기화
		if (HealthWidgetClass)
		{
			HealthBarWidgetComponent->SetWidgetClass(HealthWidgetClass);
			HealthBarWidget = Cast<UHealthBarWidget>(HealthBarWidgetComponent->GetUserWidgetObject());
			
			if (HealthBarWidget)
			{
				// 체력 UI 초기화
				UpdateHealthUI();
			}
			else
			{
				TESTLOG(Warning, TEXT("Failed to create HealthBarWidget"));
			}
		}
		else
		{
			TESTLOG(Error, TEXT("HealthWidgetClass not set"));
		}
	}
}


void APlayerCharacter::UpdateHealthUI() const
{
	if (HealthBarWidget)
	{
		// 체력 퍼센트 업데이트
		HealthBarWidget->UpdateHealthBar(GetHealth() / 100.0f);
		
		// 체력바 색상 업데이트 (자신의 캐릭터인지 여부에 따라)
		HealthBarWidget->UpdateHealthBarColor(IsLocallyControlled());
	}
}


void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &APlayerCharacter::Client_Attack);
}


void APlayerCharacter::Client_Attack()
{
	if (HasAuthority() == true)
	{
		Server_Attack();
	}
	else
	{
		Server_Attack();
	}
}







void APlayerCharacter::OnRep_Health() const
{
	// Health 값 변경 시 UI 업데이트 처리
	UpdateHealthUI();
}
