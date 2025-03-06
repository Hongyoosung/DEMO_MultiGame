// Copyright Epic Games, Inc. All Rights Reserved.

#include "DEMO_MultiGameGameMode.h"
#include "DEMO_MultiGamePlayerController.h"
#include "DEMO_MultiGameCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Processor/FInputProcessor.h"
#include "HAL/RunnableThread.h"

ADEMO_MultiGameGameMode::ADEMO_MultiGameGameMode() : InputProcessor(nullptr), InputProcessorThread(nullptr)
{
	// use our custom PlayerController class
	PlayerControllerClass = ADEMO_MultiGamePlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDown/Blueprints/BP_TopDownCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// set default controller to our Blueprinted controller
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/TopDown/Blueprints/BP_TopDownPlayerController"));
	if(PlayerControllerBPClass.Class != NULL)
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}
}

void ADEMO_MultiGameGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 스레드 생성, 시작
	InputProcessor = new FInputProcessor();
	InputProcessorThread = FRunnableThread::Create(InputProcessor, TEXT("InputProcessorThread"));

	UE_LOG(LogTemp, Log, TEXT("Starting InputProcessor"));

	// 테스트 요청 추가
	InputProcessor->AddRequest("Moving");
	InputProcessor->AddRequest("Attack");
}

void ADEMO_MultiGameGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 스레드 종료
	if (InputProcessor)
	{
		InputProcessor->Stop();
	}
	if (InputProcessorThread)
	{
		InputProcessorThread->WaitForCompletion();
		delete InputProcessorThread;
		InputProcessorThread = nullptr;
	}
	if (InputProcessor)
	{
		delete InputProcessor;
		InputProcessor = nullptr;
	}
	
	Super::EndPlay(EndPlayReason);
}
