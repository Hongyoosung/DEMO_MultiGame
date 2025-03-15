#include "TAttackTask.h"
#include "DEMO_MultiGame.h"
#include "Characters/PlayerCharacter.h"


FTAttackTask::FTAttackTask() :	 AttackerPlayerWeak(nullptr),
								CompletionCallback(nullptr), PlayerAttackRange(0.0f), 
								CachedAttackerLocation(FVector::ZeroVector),
								CachedAttackerName(TEXT(""))
{
}


void FTAttackTask::InitializePlayerValues(APlayerCharacter* InPlayer)
{
	
#ifdef UE_SERVER
	
	AttackerPlayerWeak		= InPlayer;
	PlayerAttackRange		= InPlayer->GetAttackRange();

	// 필요한 데이터 복사 (객체 의존성 줄이기)
	CachedAttackerLocation	= InPlayer->GetActorLocation();
	CachedAttackerName		= InPlayer->GetName();
	
#endif
	
}


void FTAttackTask::SetCompletionCallback(TFunction<void(FTAttackTask*)> InCallback)
{
	
#ifdef UE_SERVER
	
	CompletionCallback = MoveTemp(InCallback);
	
#endif
	
}


void FTAttackTask::DoThreadedWork()
{
	
#ifdef UE_SERVER
	
	SetTaskRunning(true);

	
	APlayerCharacter* AttackerPlayer = AttackerPlayerWeak.Get();
	if (!AttackerPlayer->IsValidLowLevel())
	{
		TESTLOG(Warning, TEXT("Player is nullptr!"));
		FinishTask();
		return;
	}

	TWeakObjectPtr<UWorld> WorldPtr = AttackerPlayer->GetWorld();
	const FVector AttackerLocation	= AttackerPlayer->GetActorLocation();
	const float AttackRange			= PlayerAttackRange;
	

	TESTLOG(Warning, TEXT("Attack executed by %s"), *AttackerPlayer->GetName());

	
	// Schedule a game thread task, while the task itself waits
	AsyncTask(ENamedThreads::GameThread, [this, AttackerLocation, AttackRange, WorldPtr]()
	{
		if (!WorldPtr.IsValid())
		{
			TESTLOG(Warning, TEXT("Invalid World in AsyncTask"));
			FinishTask();
			return;
		}

		APlayerCharacter* AttackerPlayer = AttackerPlayerWeak.Get();
		if (!IsValid(AttackerPlayer))
		{
			TESTLOG(Warning, TEXT("Invalid AttackerPlayer in AsyncTask"));
			FinishTask();
			return;
		}

		TArray<FHitResult> HitResults;
		PerformCollisionDetection(AttackerLocation, AttackRange, WorldPtr, HitResults);
		ApplyDamageToHitPlayers(HitResults);

		FinishTask();
	});
	
#endif
	
}


void FTAttackTask::Abandon()
{
	
#ifdef UE_SERVER
	
	if (IsTaskRunning())
	{
		TESTLOG(Warning, TEXT("Abandoning task %p"), this);
		SetTaskRunning(false); // 태스크 실행 상태 해제
		if (CompletionCallback)
		{
			// 게임 스레드에서 콜백을 호출하지 않고 즉시 실행
			CompletionCallback(this);
			CompletionCallback = nullptr;
		}
	}
	
#endif
	
}


void FTAttackTask::Init()
{
	
#ifdef UE_SERVER
	
	if (IsTaskRunning())
	{
		TESTLOG(Warning, TEXT("Task is still running, skipping Init"));
		return;
	}
        
	AttackerPlayerWeak.Reset();
	PlayerAttackRange = 0.0f;
	CompletionCallback = nullptr;
	HitPlayers.Empty();
	CachedAttackerLocation = FVector::ZeroVector;
	CachedAttackerName = TEXT("");
	SetReturnedToPool(true);
	SetTaskRunning(false); // 상태 재설정
	
#endif
	
}


void FTAttackTask::FinishTask()
{
	
#ifdef UE_SERVER
	
	// Avoid re-completing tasks that have already been completed
	if (!bIsTaskRunning)
	{
		return;
	}
            
	// Make sure you're in the game thread before calling the completion callback
	if (!IsInGameThread())
	{
		bIsTaskRunning = false;
		AsyncTask(ENamedThreads::GameThread, [this]()
		{
			if (CompletionCallback) 
			{
				auto TempCallback = CompletionCallback;
				CompletionCallback = nullptr; // 콜백을 한 번만 호출하도록 함
				TempCallback(this);
			}
		});
	}
	else
	{
		if (CompletionCallback)
		{
			auto TempCallback = CompletionCallback;
			CompletionCallback = nullptr; // 콜백을 한 번만 호출하도록 함
			TempCallback(this);
		}
	}
#endif
	
}


void FTAttackTask::ApplyDamageToHitPlayers(const TArray<FHitResult>& HitResults)
{
	
#ifdef UE_SERVER
	
	check(IsInGameThread());

	if (!AttackerPlayerWeak.Get() || !AttackerPlayerWeak->IsValidLowLevel())
	{
		return;
	}

	for (const auto& Hit : HitResults)
	{
		APlayerCharacter* OtherCharacter = Cast<APlayerCharacter>(Hit.GetActor());
		if (OtherCharacter && OtherCharacter != AttackerPlayerWeak.Get() && OtherCharacter->IsValidLowLevel())
		{
			HitPlayers.Add(OtherCharacter);
			
			OtherCharacter->TakeDamage(10.0f);

			TESTLOG(Warning, TEXT("Player %s hit by %s"), *OtherCharacter->GetName(), *AttackerPlayerWeak.Get()->GetName());
		}
		else
		{
			TESTLOG(Error, TEXT("Hit actor is not a valid player character!"));
		}
	}
	
#endif
	
}


void FTAttackTask::PerformCollisionDetection(const FVector& Start, float Range, TWeakObjectPtr<UWorld> WorldPtr,
	TArray<FHitResult>& OutHitResults) const
{
	
#ifdef UE_SERVER
	
	check(IsInGameThread()); // Ensuring that it is only called from the game thread
	
	if (!WorldPtr.IsValid())
	{
		TESTLOG(Warning, TEXT("World is no longer valid!"));
		return;
	}

	UWorld* World = WorldPtr.Get();
	if (!World)
	{
		return;
	}

	FCollisionQueryParams Params;
	if (AttackerPlayerWeak.Get())
	{
		Params.AddIgnoredActor(AttackerPlayerWeak.Get());
	}

	const FVector End = Start;

	
	World->SweepMultiByChannel(
		OutHitResults,
		Start,
		End,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(Range),
		Params);

	
	OutHitResults.RemoveAll([](const FHitResult& Hit)
	{
		return !Cast<APlayerCharacter>(Hit.GetActor());
	});
	
#endif
	
}
