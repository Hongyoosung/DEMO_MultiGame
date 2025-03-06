#pragma once
#include "Characters/PlayerCharacter.h"
#include "Async/AsyncWork.h"
#include "DEMO_MultiGameGameMode.h"

class FTAttackTask : public FNonAbandonableTask
{
public:
	APlayerCharacter* Player;

	FTAttackTask(APlayerCharacter* InPlayer) : Player(InPlayer){}

	void DoWork() const
	{
		if (Player)
		{
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
					Params
				);

				if (bHit)
				{
					for (auto& Hit : HitResults)
					{
						APlayerCharacter* OtherCharacter = Cast<APlayerCharacter>(Hit.GetActor());

						if (OtherCharacter && OtherCharacter != Player)
						{
							// AntiCheat: 공격 범위와 위치를 서버에서 확인하여 옳은 공격인지 검증
							const float Distance = FVector::Dist(Player->GetActorLocation(), OtherCharacter->GetActorLocation());
							if (Distance > 300.0f)
							{
								UE_LOG(LogTemp, Warning, TEXT("Attack target is too far: %f units"), Distance);
							}
							else
							{
								OtherCharacter->TakeDamage(10.0f);
							}
						}
					}
				}
			});
		}
	}

	FORCEINLINE TStatId GetStatId() const { return TStatId(); }
};
