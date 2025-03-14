// AntiCheatComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AntiCheatComponent.generated.h"


class APlayerCharacter;
class UAntiCheatManager;
class AMultiGameMode;


USTRUCT()
struct FPlayerChecksum
{
	GENERATED_BODY()
public:
	FPlayerChecksum() : HealthChecksum(0), PositionChecksum(0), LastChecksumPosition(FVector3d(0, 0, 0)) {}

	// Getters
	uint32		GetHealthChecksum		()	const					{		return HealthChecksum;			}
	uint32		GetPositionChecksum		()	const					{		return PositionChecksum;		}
	FVector		GetLastChecksumPosition	()	const					{		return LastChecksumPosition;	}
	
	// Setters
	void		SetHealthChecksum		(const uint32 Checksum)		{		HealthChecksum = Checksum;			}
	void		SetPositionChecksum		(const uint32 Checksum)		{		PositionChecksum = Checksum;		}
	void		SetLastChecksumPosition	(const FVector& Position)	{		LastChecksumPosition = Position;	}


private:
	UPROPERTY()
	uint32 HealthChecksum;

	UPROPERTY()
	uint32 PositionChecksum;

	UPROPERTY()
	FVector LastChecksumPosition;
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DEMO_MULTIGAME_API UAntiCheatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAntiCheatComponent();

	void InitializeGameMode(AMultiGameMode* InGameMode);
	
	FPlayerChecksum GetChecksums() const { return Checksums; }

	void UpdateAllChecksums();

	// 액션 전 기본 검증 (로컬 검증 후 필요시 매니저에 위임)
	bool ValidatePlayerForAction() const;
    
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

	UPROPERTY()
	AMultiGameMode* GameMode;
	
	UPROPERTY()
	FPlayerChecksum Checksums;
};

// AntiCheatManager.h - 기존 코드 유지
// 주요 변경점: 컴포넌트와의 협력을 위한 인터페이스 추가