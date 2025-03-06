// Copyright Epic Games, Inc. All Rights Reserved.

#include "DEMO_MultiGameGameMode.h"
#include "DEMO_MultiGamePlayerController.h"
#include "DEMO_MultiGameCharacter.h"
#include "UObject/ConstructorHelpers.h"

ADEMO_MultiGameGameMode::ADEMO_MultiGameGameMode()
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