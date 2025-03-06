#pragma once
#include "Characters/PlayerCharacter.h"
#include "Async/AsyncWork.h"
#include "AntiCheat/AntiCheatManager.h"
#include "DEMO_MultiGameGameMode.h"

class FTAttackTask : public FNonAbandonableTask
{
public:
	APlayerCharacter* Player;

	FTAttackTask(APlayerCharacter* InPlayer) : Player(InPlayer){}

	void DoWork() const
	{
		if (Player == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Player is nullptr!"));
		}
		
		// 공격 로직 실행
		UE_LOG(LogTemp, Log, TEXT("Attack executed by %s"), *Player->GetName());

		// 실제 공격 로직
		// 충돌 검사 및 데미지 처리
		AsyncTask(ENamedThreads::GameThread, [this]()
		{
			TArray<FHitResult> HitResults;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(Player); // Player를 무시

			const FVector Start = Player->GetActorLocation();
			const FVector End = Start;

			const bool bHit = Player->GetWorld()->SweepMultiByChannel(
			HitResults,
				Start,
				End,
				FQuat::Identity,
				ECC_Pawn,
				FCollisionShape::MakeSphere(300.0f),
				Params);

			if (bHit)
			{
				for (auto& Hit : HitResults)
				{
					APlayerCharacter* OtherCharacter = Cast<APlayerCharacter>(Hit.GetActor());

					if (OtherCharacter && OtherCharacter != Player)
					{
						constexpr float Distance = 300.0f;

						// 공격 범위 검증
						if (UAntiCheatManager::GetInstance()->VerifyAttackRange(Player, OtherCharacter, Distance))
						{
							OtherCharacter->TakeDamage(10.0f);
						}
						else
						{
							return;
						}
					}
				}
			}
		});
	}
	

	FORCEINLINE TStatId GetStatId() const { return TStatId(); }
};
