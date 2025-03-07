#pragma once
#include "DEMO_MultiGame.h"
#include "Characters/PlayerCharacter.h"
#include "Async/AsyncWork.h"
#include "Tasks/PoolableQueuedWork.h"
#include "AntiCheat/AntiCheatManager.h"

class FTAttackTask : public FPoolableQueuedWork
{
public:
    FTAttackTask() : AttackerPlayer(nullptr), PlayerAttackRange(0.0f) {}

    void InitializePlayerValues(APlayerCharacter* InPlayer)
    {
        AttackerPlayer = InPlayer;
        PlayerAttackRange = InPlayer->GetAttackRange();
    }

    void SetCompletionCallback(TFunction<void(FTAttackTask*)> InCallback)
    {
        CompletionCallback = MoveTemp(InCallback);
    }

    virtual void DoThreadedWork() override
    {
        if (!AttackerPlayer->IsValidLowLevel())
        {
            TESTLOG(Warning, TEXT("Player is nullptr!"));
            if (CompletionCallback) CompletionCallback(this);
            return;
        }

        bIsTaskRunning = true;
        const FVector AttackerLocation = AttackerPlayer->GetActorLocation();
        const float AttackRange = PlayerAttackRange;
        TWeakObjectPtr<UWorld> WorldPtr = AttackerPlayer->GetWorld();

        TESTLOG(Warning, TEXT("Attack executed by %s"), *AttackerPlayer->GetName());

        AsyncTask(ENamedThreads::GameThread, [this, AttackerLocation, AttackRange, WorldPtr]()
        {
            if (!WorldPtr.IsValid() || !AttackerPlayer->IsValidLowLevel())
            {
                TESTLOG(Warning, TEXT("Invalid World or AttackerPlayer"));
                if (CompletionCallback) CompletionCallback(this);
                bIsTaskRunning = false;
                return;
            }

            TArray<FHitResult> HitResults;
            PerformCollisionDetection(AttackerLocation, AttackRange, WorldPtr, HitResults);
            ApplyDamageToHitPlayers(HitResults);

            if (CompletionCallback) CompletionCallback(this);
            bIsTaskRunning = false;
        });
    }

    virtual void Abandon() override {}

    virtual void Init() override
    {
        if (bIsTaskRunning)
        {
            TESTLOG(Warning, TEXT("Task is still running, skipping Init"));
            return;
        }
        
        AttackerPlayer->Reset();
        PlayerAttackRange = 0.0f;
        CompletionCallback = nullptr;
        HitPlayers.Empty();
        SetReturnedToPool(true);
    }

private:
    void ApplyDamageToHitPlayers(const TArray<FHitResult>& HitResults)
    {
        check(IsInGameThread());

        if (!AttackerPlayer || !AttackerPlayer->IsValidLowLevel())
        {
            return;
        }

        for (const auto& Hit : HitResults)
        {
            APlayerCharacter* OtherCharacter = Cast<APlayerCharacter>(Hit.GetActor());
            if (OtherCharacter && OtherCharacter != AttackerPlayer && OtherCharacter->IsValidLowLevel())
            {
                HitPlayers.Add(OtherCharacter);
                OtherCharacter->TakeDamage(10.0f);

                TESTLOG(Warning, TEXT("Player %s hit by %s"), *OtherCharacter->GetName(), *AttackerPlayer->GetName());
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
        if (AttackerPlayer)
        {
            Params.AddIgnoredActor(AttackerPlayer);
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
    }

private:
    APlayerCharacter* AttackerPlayer;
    TArray<APlayerCharacter*> HitPlayers;
    TFunction<void(FTAttackTask*)> CompletionCallback;
    float PlayerAttackRange;
};