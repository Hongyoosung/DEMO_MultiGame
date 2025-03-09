// AntiCheatComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AntiCheatComponent.generated.h"

class APlayerCharacter;
class UAntiCheatManager;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DEMO_MULTIGAME_API UAntiCheatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAntiCheatComponent();

	// 체크섬 업데이트 메서드
	void UpdateAllChecksums();
    
	// 액션 전 기본 검증 (로컬 검증 후 필요시 매니저에 위임)
	bool ValidatePlayerForAction() const;
    
	// 체크섬 업데이트를 위한 틱 처리
	void TickChecksumUpdate(float DeltaTime);
    
	// 공격 범위 검증을 위한 헬퍼 메서드
	bool IsTargetInRange(const APlayerCharacter* Target) const;

protected:
	virtual void BeginPlay() override;

private:
	// 안티 치트 매니저에 대한 참조 획득
	UAntiCheatManager* GetAntiCheatManager() const;
    
	// 컴포넌트 설정
	UPROPERTY(EditDefaultsOnly, Category = "AntiCheat")
	float ChecksumUpdateInterval;

	float TimeSinceLastChecksumUpdate;
    
	UPROPERTY()
	APlayerCharacter* OwnerCharacter;
};

// AntiCheatManager.h - 기존 코드 유지
// 주요 변경점: 컴포넌트와의 협력을 위한 인터페이스 추가