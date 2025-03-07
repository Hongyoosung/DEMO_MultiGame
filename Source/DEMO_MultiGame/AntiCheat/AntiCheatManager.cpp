#include "AntiCheatManager.h"
#include "Characters/PlayerCharacter.h"
#include "Misc/Crc.h"
#include "DEMO_MultiGame.h"


UAntiCheatManager::UAntiCheatManager()
	: FailedChecksumCount(0), PositionToleranceThreshold(1000.0f)
{
}


UAntiCheatManager::~UAntiCheatManager()
{
	// Log stats if desired
	TESTLOG(Warning, TEXT("AntiCheatManager: Total failed checksums: %d"), FailedChecksumCount);
}


UAntiCheatManager* UAntiCheatManager::CreateManager()
{
	UAntiCheatManager* Manager = NewObject<UAntiCheatManager>();
	
	return Manager;
}



bool UAntiCheatManager::VerifyPositionWithTolerance(const APlayerCharacter* Player) const
{
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
}


bool UAntiCheatManager::VerifyAttackRange(const APlayerCharacter* Attacker, const APlayerCharacter* Target, const float MaxRange) const
{
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
}


uint32 UAntiCheatManager::CalculateHealthChecksum(const float Health) const
{
	return FCrc::MemCrc32(&Health, sizeof(Health));
}


uint32 UAntiCheatManager::CalculatePositionChecksum(const FVector& Position) const
{
	return FCrc::MemCrc32(&Position, sizeof(FVector));
}


bool UAntiCheatManager::VerifyHealthChecksum(const APlayerCharacter* Player) const
{
	if (!Player)
	{
		TESTLOG(Warning, TEXT("VerifyHealthChecksum: Player is null"));
		return false;
	}

	// Calculate current checksum
	const uint32 CalculatedChecksum = CalculateHealthChecksum(Player->GetHealth());
	
	// Get stored checksum
	const uint32 StoredChecksum = Player->GetChecksums().GetHealthChecksum();

	TESTLOG(Log, TEXT("VerifyHealthChecksum: Player=%s, Health=%.2f, CalculatedChecksum=%u, StoredChecksum=%u"),
		*Player->GetName(), Player->GetHealth(), CalculatedChecksum, StoredChecksum);

	const bool bIsValid = (StoredChecksum == CalculatedChecksum);

	if (!bIsValid)
	{
		TESTLOG(Warning, TEXT("VerifyHealthChecksum: Checksum mismatch for Player %s"), *Player->GetName());
		const_cast<UAntiCheatManager*>(this)->FailedChecksumCount++;
	}

	return bIsValid;
}


bool UAntiCheatManager::VerifyPositionChecksum(const APlayerCharacter* Player) const
{
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
}


bool UAntiCheatManager::VerifyPlayerValid(const APlayerCharacter* Player) const
{
	return Player && Player->IsValidLowLevel() && !Player->IsPendingKillPending() && Player->GetHealth() > 0;
}

bool UAntiCheatManager::VerifyAllChecksums(const APlayerCharacter* Player) const
{
	// Verify all checksums at once
	return VerifyHealthChecksum(Player) && VerifyPositionChecksum(Player);
}
