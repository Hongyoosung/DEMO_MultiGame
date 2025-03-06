#pragma once
#include "DEMO_MultiGame.h"
#include "Characters/PlayerCharacter.h"
#include "Async/AsyncWork.h"
#include "AntiCheat/AntiCheatManager.h"


class FTAttackTask : public IQueuedWork
{
public:
	FTAttackTask() : Player(nullptr) {}

	void Initialize(APlayerCharacter* InPlayer)
	{
		Player = InPlayer;
	}

	void SetCompletionCallback(TFunction<void(FTAttackTask*)> InCallback)
	{
		CompletionCallback = MoveTemp(InCallback);
	}

	virtual void DoThreadedWork() override
	{
		if (Player == nullptr)
		{
			TESTLOG(Warning, TEXT("Player is nullptr!"));
		}

		TESTLOG(Warning, TEXT("Attack eecuted by %s "), *Player->GetName());

		// 게임 스레드에서 실행해야 하는 작업
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

	virtual void Abandon() override {};

private:
	void PerformAttackLogic() const
	{
		if (Player == nullptr || Player->IsValidLowLevel() == false)
		{
			TESTLOG(Warning, TEXT("Player is nullptr!"));
			return;
		}
		
		TArray<FHitResult> HitResults;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(Player);

		const FVector Start = Player->GetActorLocation();
		const FVector End = Start;
		constexpr float Distance = 300.0f;

		const bool bHit = Player->GetWorld()->SweepMultiByChannel(
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

			if (OtherCharacter && OtherCharacter != Player)
			{
				if (UAntiCheatManager::GetInstance()->VerifyAttackRange(Player, OtherCharacter, Distance))
				{
					OtherCharacter->TakeDamage(10.0f);
				}
			}
		}

		
	}
	
private:
	APlayerCharacter* Player;
	TFunction<void(FTAttackTask*)> CompletionCallback;
};
