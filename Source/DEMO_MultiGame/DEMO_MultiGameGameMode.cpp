// Copyright Epic Games, Inc. All Rights Reserved.

#include "DEMO_MultiGameGameMode.h"
#include "DEMO_MultiGamePlayerController.h"
#include "UObject/ConstructorHelpers.h"


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
}





