#pragma once
#include "DEMO_MultiGame.h"
#include "Characters/PlayerCharacter.h"
#include "Async/AsyncWork.h"
#include "Tasks/PoolableQueuedWork.h"
#include "AntiCheat/AntiCheatManager.h"

class FTAttackTask : public FPoolableQueuedWork
{
public:
    FTAttackTask() : PlayerAttackRange(0.0f) {}

    void InitializePlayerValues(APlayerCharacter* InPlayer)
    {
        AttackerPlayerWeak = InPlayer;
        PlayerAttackRange = InPlayer->GetAttackRange();

        // 필요한 데이터 복사 (객체 의존성 줄이기)
        CachedAttackerLocation = InPlayer->GetActorLocation();
        CachedAttackerName = InPlayer->GetName();
    }

    void SetCompletionCallback(TFunction<void(FTAttackTask*)> InCallback)
    {
        CompletionCallback = MoveTemp(InCallback);
    }

    virtual void DoThreadedWork() override
    {
        SetTaskRunning(true);
        
        APlayerCharacter* AttackerPlayer = AttackerPlayerWeak.Get();
        if (!AttackerPlayer->IsValidLowLevel())
        {
            TESTLOG(Warning, TEXT("Player is nullptr!"));
            FinishTask();
            return;
        }

        
        
        const FVector AttackerLocation = AttackerPlayer->GetActorLocation();
        const float AttackRange = PlayerAttackRange;
        TWeakObjectPtr<UWorld> WorldPtr = AttackerPlayer->GetWorld();

        TESTLOG(Warning, TEXT("Attack executed by %s"), *AttackerPlayer->GetName());

        // 게임 스레드 작업을 예약하고 태스크 자체는 대기
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
    }

    virtual void Abandon() override {}

    virtual void Init() override
    {
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
    }

private:
    void FinishTask()
    {
        // 이미 완료된 태스크를 다시 완료하지 않도록 함
        if (!bIsTaskRunning)
        {
            SetTaskRunning(false);
            
            // 완료 콜백을 호출하기 전에 게임 스레드에 있는지 확인
            if (!IsInGameThread())
            {
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
        }
    }

    
    void ApplyDamageToHitPlayers(const TArray<FHitResult>& HitResults)
    {
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
    }

    void PerformCollisionDetection(const FVector& Start, float Range, TWeakObjectPtr<UWorld> WorldPtr, TArray<FHitResult>& OutHitResults) const
    {
        check(IsInGameThread()); // 게임 스레드에서만 호출되도록 보장

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

        // 구체 형태로 충돌 검사
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
    }

private:
    TWeakObjectPtr<APlayerCharacter> AttackerPlayerWeak;
    TArray<APlayerCharacter*> HitPlayers;
    TFunction<void(FTAttackTask*)> CompletionCallback;
    float PlayerAttackRange;

    // 캐시된 데이터
    FVector CachedAttackerLocation;
    FString CachedAttackerName;
};