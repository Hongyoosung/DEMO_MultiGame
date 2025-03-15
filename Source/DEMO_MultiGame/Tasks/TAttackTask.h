#pragma once
#include "Async/AsyncWork.h"
#include "Tasks/PoolableQueuedWork.h"


class APlayerCharacter;


class FTAttackTask final : public FPoolableQueuedWork
{
public:
    FTAttackTask();

    void InitializePlayerValues(APlayerCharacter* InPlayer);

    virtual void DoThreadedWork ()  override;
    virtual void Abandon        ()  override;
    virtual void Init           ()  override;
    
    void SetCompletionCallback(TFunction<void(FTAttackTask*)> InCallback);
    

private:
    void FinishTask();
    void ApplyDamageToHitPlayers    (   const TArray<FHitResult>& HitResults);
    void PerformCollisionDetection  (   const FVector& Start, float Range,
                                        TWeakObjectPtr<UWorld> WorldPtr,
                                        TArray<FHitResult>& OutHitResults)
                                        const;

    
private:
    TWeakObjectPtr<APlayerCharacter>    AttackerPlayerWeak;
    TArray<APlayerCharacter*>           HitPlayers;
    TFunction<void(FTAttackTask*)>      CompletionCallback;
    
    float PlayerAttackRange;

    // 캐시된 데이터
    FVector CachedAttackerLocation;
    FString CachedAttackerName;
};