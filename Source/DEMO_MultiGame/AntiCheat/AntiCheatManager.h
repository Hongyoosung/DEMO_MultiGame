#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AntiCheatManager.generated.h"

class APlayerCharacter;

UCLASS()
class UAntiCheatManager : public UObject
{
	GENERATED_BODY()
	
public:
	UAntiCheatManager();
	virtual ~UAntiCheatManager() override;

	
	static UAntiCheatManager* CreateManager();
	

	// Verification methods
	bool VerifyAttackRange				(const APlayerCharacter* Attacker, const APlayerCharacter* Target, const float MaxRange) const;
	bool VerifyHealthChecksum			(const APlayerCharacter* Player) const;
	bool VerifyPositionChecksum			(const APlayerCharacter* Player) const;
	bool VerifyPlayerValid				(const APlayerCharacter* Player) const;
	bool VerifyAllChecksums				(const APlayerCharacter* Player) const;


private:
	// On-the-go checksum verification mitigation logic
	bool VerifyPositionWithTolerance(const APlayerCharacter* Player) const;

	
private:
	// Helper methods for checksum calculation
	uint32 CalculateHealthChecksum		(const float Health)		const;
	uint32 CalculatePositionChecksum	(const FVector& Position)	const;

	// For performance logging
	UPROPERTY()
	int32 FailedChecksumCount;

	// Tolerance thresholds for position verification
	UPROPERTY(EditDefaultsOnly, Category = "AntiCheat")
	float PositionToleranceThreshold;
};
