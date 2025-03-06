#include "FInputProcessor.h"

FInputProcessor::FInputProcessor()
{
	bIsRunning = true;
}

FInputProcessor::~FInputProcessor()
{
}

bool FInputProcessor::Init()
{
	// 스레드가 시작되기 전에 호출, 초기화 작업 필요 시 작성
	
	return true;
}

uint32 FInputProcessor::Run()
{
	// 클라이언트 입력 처리, 큐에서 요청을 꺼내 처리하는 등
	while (bIsRunning)
	{
		FString Request;
		{
			// 뮤텍스로 큐 보호
			FScopeLock Lock(&QueueMutex);
			if (!RequestQueue.IsEmpty())
			{
				Request = RequestQueue[0];
				RequestQueue.RemoveAt(0); // 첫 번째 요청 처리
			}
		}

		if (!Request.IsEmpty())
		{
			// 요청 처리
			UE_LOG(LogTemp, Log, TEXT("Processing Request: %s"), *Request);
		}
		
		FPlatformProcess::Sleep(0.1f);
	}

	return 0;
}

void FInputProcessor::Stop()
{
	// 스레드 종료 시 호출
	bIsRunning = false;
}

void FInputProcessor::AddRequest(const FString& Request)
{
	FScopeLock Lock(&QueueMutex);  // 뮤텍스 잠금
	RequestQueue.Add(Request);     // 큐에 요청 추가
}
