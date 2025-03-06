#pragma once
#include "DEMO_MultiGame.h"
#include "Characters/PlayerCharacter.h"
#include "Async/AsyncWork.h"
#include "AntiCheat/AntiCheatManager.h"


class FTAttackTask : public IQueuedWork
{
public:
	FTAttackTask() : AttackerPlayer(nullptr) {}

	void Initialize(APlayerCharacter* InPlayer)
	{
		AttackerPlayer = InPlayer;
	}

	// 완료 콜백 설정
	void SetCompletionCallback(TFunction<void(FTAttackTask*)> InCallback)
	{
		CompletionCallback = MoveTemp(InCallback);
	}

	// 스레드에서 실행될 작업 함수
	virtual void DoThreadedWork() override
	{
		if (AttackerPlayer == nullptr)
		{
			TESTLOG(Warning, TEXT("Player is nullptr!"));
		}

		TESTLOG(Warning, TEXT("Attack eecuted by %s "), *AttackerPlayer->GetName());

		// 게임 스레드에서 실행
		AsyncTask(ENamedThreads::GameThread, [this]()
		{
			PerformAttackLogic();
		});

		// 작업 완료 후 콜백 실행
		if (CompletionCallback)
		{
			AsyncTask(ENamedThreads::GameThread, [this]()
			{
				CompletionCallback(this);
			});
		}
	}

	// 작업 중단
	virtual void Abandon() override {};

private:
	void PerformAttackLogic() const
	{
		if (AttackerPlayer == nullptr || AttackerPlayer->IsValidLowLevel() == false)
		{
			TESTLOG(Warning, TEXT("Player is nullptr!"));
			return;
		}
		
		TArray<FHitResult> HitResults;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(AttackerPlayer);

		const FVector Start = AttackerPlayer->GetActorLocation();
		const FVector End = Start;
		constexpr float Distance = 300.0f;

		// 구체 형태로 충돌 검사
		const bool bHit = AttackerPlayer->GetWorld()->SweepMultiByChannel(
			HitResults,
			Start,
			End,
			FQuat::Identity,
			ECC_Pawn,
			FCollisionShape::MakeSphere(Distance),
			Params);

		if (bHit == false)
		{
			TESTLOG(Warning, TEXT("HitResult is null!"));
			return;
		}
		
		for (auto& Hit : HitResults)
		{
			APlayerCharacter* OtherCharacter = Cast<APlayerCharacter>(Hit.GetActor());

			if (OtherCharacter && OtherCharacter != AttackerPlayer)
			{
				// 공격 범위 검증
				if (UAntiCheatManager::GetInstance()->VerifyAttackRange(AttackerPlayer, OtherCharacter, Distance))
				{
					OtherCharacter->TakeDamage(10.0f);
				}
			}
		}

		
	}
	
private:
	APlayerCharacter* AttackerPlayer;
	TFunction<void(FTAttackTask*)> CompletionCallback;
};
