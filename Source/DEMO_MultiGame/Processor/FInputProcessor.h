#pragma once
#include "CoreMinimal.h"
#include "HAL/Runnable.h"

class FInputProcessor : public FRunnable
{
public:
	FInputProcessor();
	virtual ~FInputProcessor();

	// FRunnable 인터페이스 구현
	virtual bool		Init() override;
	virtual uint32		Run() override;
	virtual void		Stop() override;

	void				AddRequest(const FString& Request);
	
private:
	bool bIsRunning;				// 스레드 실행 상태 플래그
	TArray<FString> RequestQueue;	// 요청 큐
	FCriticalSection QueueMutex;	// 큐 보호를 위한 뮤텍스(락)
};
