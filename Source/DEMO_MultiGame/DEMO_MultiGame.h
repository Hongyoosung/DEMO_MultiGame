// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDEMO_MultiGame, Log, All);

DECLARE_LOG_CATEGORY_EXTERN(PlayerCharacter, Log, All);

// 로그를 출력한 함수 이름과 코드라인 번호 출력
#define TESTLOG_CALLINFO (FString(__FUNCTION__) + TEXT("(") + FString::FromInt(__LINE__) + TEXT(")"))
#define TESTLOG_S(Verbosity) UE_LOG(PlayerCharacter, Verbosity, TEXT("%s"), *TESTLOG_CALLINFO)
#define TESTLOG(Verbosity, Format, ...) UE_LOG(PlayerCharacter, Verbosity, TEXT("%s %s"), *TESTLOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))

