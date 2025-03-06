// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DEMO_MultiGameGameMode.generated.h"

class FInputProcessor;
class FRunnableThread;

UCLASS(minimalapi)
class ADEMO_MultiGameGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADEMO_MultiGameGameMode();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	FInputProcessor* InputProcessor;
	FRunnableThread* InputProcessorThread;
};



