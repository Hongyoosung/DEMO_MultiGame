// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "PlayerCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/HealthBarWidget.h"
#include "Tasks/TAttackTask.h"
#include "AntiCheat/AntiCheatManager.h"
#include "Modules/ModuleManager.h"


DEFINE_LOG_CATEGORY(PlayerCharacter);
IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, PlayerCharacter, "PlayerCharacter");


APlayerCharacter::APlayerCharacter() : Health(100.0f)
{
	HealthBarWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	HealthBarWidgetComponent->SetupAttachment(GetMesh());
	HealthBarWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f));
	HealthBarWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen); 
	HealthBarWidgetComponent->SetDrawSize(FVector2D(200.0f, 50.0f));
	HealthBarWidgetComponent->SetNetAddressable();
	HealthBarWidgetComponent->SetIsReplicated(true);

	UAntiCheatManager::GetInstance()->UpdateHealthChecksum(this);
}


void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeHealthWidget();
	
	UAntiCheatManager::GetInstance()->UpdateHealthChecksum(this);
}


void APlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APlayerCharacter, Health);
}


void APlayerCharacter::Multicast_SpawnHitEffect_Implementation(const FVector Location)
{
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

		// Health 값 체크섬 업데이트
		UAntiCheatManager::GetInstance()->UpdateHealthChecksum(this);

		// 각 클라이언트가 이펙트를 표출하게 함
		Multicast_SpawnHitEffect(GetActorLocation());
	}
}


bool APlayerCharacter::Server_Attack_Validate()
{
	return true;
}

void APlayerCharacter::Server_Attack_Implementation()
{
	// 체크섬 검사
	bool bIsValidChecksum = UAntiCheatManager::GetInstance()->VerifyHealthChecksum(this);
	
	if (bIsValidChecksum == false)
	{
		TESTLOG(Warning, TEXT("Health checksum verification failed. Possible memory tampering detected."));
		return;
	}

	
	// 비동기 형식으로 작업 생성 및 자동 삭제
	(new FAutoDeleteAsyncTask<FTAttackTask>(this))->StartBackgroundTask();
	
	// 동기 형식
	/*
	// FAsyncTask로 비동기 작업 생성
	FAsyncTask<FTAttackTask>* Task = new FAsyncTask<FTAttackTask>(this);
	Task->StartBackgroundTask();

	// 작업 완료 후 삭제
	Task->EnsureCompletion(); // 동기적으로 완료 대기 (실제로는 비동기 처리 필요)
	delete Task;
	*/
}


void APlayerCharacter::OnRep_Health() const
{
	// Health 값 변경 시 UI 업데이트 처리
	UpdateHealthUI();
}
