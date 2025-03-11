#include "AntiCheatManager.h"
#include "Characters/PlayerCharacter.h"
#include "Misc/Crc.h"
#include "DEMO_MultiGame.h"
#include "Components/HealthComponent.h"


UAntiCheatManager::UAntiCheatManager()
	: FailedChecksumCount(0), PositionToleranceThreshold(1000.0f), MaxAllowedSpeed(600.0f)
{
}


UAntiCheatManager::~UAntiCheatManager()
{
#ifdef UE_SERVER
	// Log stats if desired
	TESTLOG(Warning, TEXT("AntiCheatManager: Total failed checksums: %d"), FailedChecksumCount);
#endif
}


bool UAntiCheatManager::VerifyPositionWithTolerance(const APlayerCharacter* Player) const
{
#ifdef UE_SERVER
	const FVector CurrentPosition = Player->GetActorLocation();
	const FVector LastChecksumPosition = Player->GetChecksums().GetLastChecksumPosition();
    
	// Calculate the distance between the current location and the last checksum location
	const float Distance = FVector::Dist(CurrentPosition, LastChecksumPosition);
    
	// Verify that the distance is within an acceptable range
	if (Distance <= PositionToleranceThreshold)
	{
		return true;
	}
    
	// Distances that exceed the allowable range are considered checksum failures
	TESTLOG(Warning, TEXT("VerifyPositionWithTolerance: Position difference too large (%.2f) for Player %s"), 
		Distance, *Player->GetName());
	
	const_cast<UAntiCheatManager*>(this)->FailedChecksumCount++;
	
	return false;
#else
	return true;
#endif
}


bool UAntiCheatManager::VerifyAttackRange(const APlayerCharacter* Attacker, const APlayerCharacter* Target, const float MaxRange) const
{
#ifdef UE_SERVER
	if (!Attacker || !Target) 
	{
		return false;
	}

	const float Distance = FVector::Dist(Attacker->GetActorLocation(), Target->GetActorLocation());
	if (Distance > MaxRange)
	{
		TESTLOG(Warning, TEXT("Attack target is too far: %f units"), Distance);
		return false;
	}

	return true;
#else
	return true;
#endif
}


uint32 UAntiCheatManager::CalculateHealthChecksum(const float Health) const
{
#ifdef UE_SERVER
	return FCrc::MemCrc32(&Health, sizeof(Health));
#else
	return 0;
#endif
}


uint32 UAntiCheatManager::CalculatePositionChecksum(const FVector& Position) const
{
#ifdef UE_SERVER
	return FCrc::MemCrc32(&Position, sizeof(FVector));
#else
	return 0;
#endif
}


bool UAntiCheatManager::VerifyHealthChecksum(const APlayerCharacter* Player) const
{
#ifdef UE_SERVER
	if (!Player)
	{
		TESTLOG(Warning, TEXT("VerifyHealthChecksum: Player is null"));
		return false;
	}

	// Calculate current checksum
	const uint32 CalculatedChecksum = CalculateHealthChecksum(Player->GetHealthComponent()->GetHealth());
	
	// Get stored checksum
	const uint32 StoredChecksum = Player->GetChecksums().GetHealthChecksum();

	TESTLOG(Log, TEXT("VerifyHealthChecksum: Player=%s, Health=%.2f, CalculatedChecksum=%u, StoredChecksum=%u"),
		*Player->GetName(), Player->GetHealthComponent()->GetHealth(), CalculatedChecksum, StoredChecksum);

	const bool bIsValid = (StoredChecksum == CalculatedChecksum);

	if (!bIsValid)
	{
		TESTLOG(Warning, TEXT("VerifyHealthChecksum: Checksum mismatch for Player %s"), *Player->GetName());
		const_cast<UAntiCheatManager*>(this)->FailedChecksumCount++;
	}

	return bIsValid;
#else
	return true;
#endif
}


bool UAntiCheatManager::VerifyPositionChecksum(const APlayerCharacter* Player) const
{
#ifdef UE_SERVER
	if (!Player)
	{
		TESTLOG(Warning, TEXT("VerifyPositionChecksum: Player is null"));
		return false;
	}


	// Check Player is Moving
	if (Player->GetVelocity().SizeSquared() > 0.0f)
	{
		// If player is moving, verify with tolerance
		return VerifyPositionWithTolerance(Player);
	}

	// if player is not moving, verify exact position
	const FVector Position = Player->GetActorLocation();
	const uint32 CalculatedChecksum = CalculatePositionChecksum(Position);
	const uint32 StoredChecksum = Player->GetChecksums().GetPositionChecksum();

	const bool bIsValid = (StoredChecksum == CalculatedChecksum);

	if (!bIsValid)
	{
		TESTLOG(Warning, TEXT("VerifyPositionChecksum: Checksum mismatch for Player %s"), *Player->GetName());
		const_cast<UAntiCheatManager*>(this)->FailedChecksumCount++;
	}

	return bIsValid;
#else
	return true;
#endif
}


bool UAntiCheatManager::VerifyPlayerValid(const APlayerCharacter* Player) const
{
#ifdef UE_SERVER
	return Player && Player->IsValidLowLevel() && !Player->IsPendingKillPending() && Player->GetHealthComponent()->GetHealth() > 0;
#else
	return true;
#endif
}

bool UAntiCheatManager::VerifyAllChecksums(const APlayerCharacter* Player) const
{
#ifdef UE_SERVER
	// Verify all checksums at once
	return VerifyHealthChecksum(Player) && VerifyPositionChecksum(Player);
#else
	return true;
#endif
}
