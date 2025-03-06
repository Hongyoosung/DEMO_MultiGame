// Copyright Epic Games, Inc. All Rights Reserved.

#include "DEMO_MultiGameGameMode.h"
#include "DEMO_MultiGamePlayerController.h"
#include "DEMO_MultiGameCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "HAL/ThreadManager.h"

ADEMO_MultiGameGameMode::ADEMO_MultiGameGameMode()
{
	PlayerControllerClass = ADEMO_MultiGamePlayerController::StaticClass();
	
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Player/BP_Player"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/TopDown/Blueprints/BP_TopDownPlayerController"));
	if(PlayerControllerBPClass.Class != NULL)
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}
}

void ADEMO_MultiGameGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 스레드 풀 초기화
	ThreadPool = FQueuedThreadPool::Allocate();
	ThreadPool->Create(4, 32 * 1024); // 4개 스레드, 32kb 크기의 스택
	UE_LOG(LogTemp, Warning, TEXT("ThreadPool initialized with 4 threads"));
	
}

void ADEMO_MultiGameGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 스레드 풀 정리
	if (ThreadPool)
	{
		ThreadPool->Destroy();
		delete ThreadPool;
		ThreadPool = nullptr;
		UE_LOG(LogTemp, Warning, TEXT("ThreadPool destroyed."));
	}
	
	Super::EndPlay(EndPlayReason);
}
