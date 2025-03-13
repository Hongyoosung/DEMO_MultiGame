#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AntiCheatManager.generated.h"

class APlayerCharacter;

UCLASS()
class DEMO_MULTIGAME_API UAntiCheatManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UAntiCheatManager();
	virtual ~UAntiCheatManager() override;
	
	// Verification methods
	bool VerifyAttackRange				(const APlayerCharacter* Attacker, const APlayerCharacter* Target, const float MaxRange) const;
	bool VerifyPlayerValid				(const APlayerCharacter* Player)				const;
	bool VerifyHealthChecksum			(const APlayerCharacter* Player)				const;
	bool VerifyPositionChecksum			(const APlayerCharacter* Player)				const;
	bool VerifyAllChecksums				(const APlayerCharacter* Player)				const;
	bool VerifyItemUsage				(const APlayerCharacter* Player, int32 ItemID)	const;
	

private:
	// Helper methods for checksum calculation
	uint32 CalculateHealthChecksum		(const float Health)		const;
	uint32 CalculatePositionChecksum	(const FVector& Position)	const;

	UPROPERTY(EditDefaultsOnly, Category = "AntiCheat")
	float MaxAllowedSpeed;
	
	// Tolerance thresholds for position verification
	UPROPERTY(EditDefaultsOnly, Category = "AntiCheat")
	float PositionToleranceThreshold;

	// For performance logging
	int32 FailedChecksumCount;
};
