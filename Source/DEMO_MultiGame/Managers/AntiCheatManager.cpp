#include "AntiCheatManager.h"
#include "Characters/PlayerCharacter.h"
#include "Misc/Crc.h"
#include "DEMO_MultiGame.h"
#include "Components/HealthComponent.h"
#include "Components/AntiCheatComponent.h"
#include "Components/InvenComponent.h"
#include "Tables/ItemData.h"


UAntiCheatManager::UAntiCheatManager()
	: MaxAllowedSpeed(600.0f), PositionToleranceThreshold(1000.0f), FailedChecksumCount(0)
{
}


UAntiCheatManager::~UAntiCheatManager()
{
#ifdef UE_SERVER
	// Log stats if desired
	TESTLOG(Warning, TEXT("AntiCheatManager: Total failed checksums: %d"), FailedChecksumCount);
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
	const uint32 StoredChecksum = Player->GetAntiCheatComponent()->GetChecksums().GetPositionChecksum();

	
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

	
	const	uint32			CurrentChecksum		= CalculatePositionChecksum(Player->GetActorLocation());
	const	uint32			StoredChecksums		= Player->GetAntiCheatComponent()->GetChecksums().GetPositionChecksum();
	
	// if the checksums match, return true
	if (CurrentChecksum == StoredChecksums)
	{
		return true;
	}

	// if not, check the real distance
	const			FVector	CurrentPosition		= Player->GetActorLocation();
	const			FVector	StoredPosition		= Player->GetAntiCheatComponent()->GetChecksums().GetLastChecksumPosition();
	
	const			float	Distance			= FVector::Dist(CurrentPosition, StoredPosition);
	constexpr		float	Threshold			= 300.0f; 

	
	const bool bIsValid = (Distance <= Threshold);

	if (!bIsValid)
	{
		TESTLOG(Warning, TEXT("VerifyPositionChecksum: Checksum mismatch for Player %s. Distance: %f (Threshold: %f)"),
				*Player->GetName(), Distance, Threshold);
		const_cast<UAntiCheatManager*>(this)->FailedChecksumCount++;
	}
	else
	{
		TESTLOG(Log, TEXT("VerifyPositionChecksum: Checksum mismatch but within threshold for Player %s. Distance: %f"),
				*Player->GetName(), Distance);
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


bool UAntiCheatManager::VerifyItemUsage(const APlayerCharacter* Player, int32 ItemID) const
{
#ifdef UE_SERVER
	if (!Player || !Player->IsValidLowLevel())
	{
		return false;
	}

	for (const FItemData& Item : Player->GetInvenComponent()->GetItemDataList())
	{
		if (Item.ItemID == ItemID)
		{
			return VerifyAllChecksums(Player); // 체크섬 검증 추가
		}
	}
	TESTLOG(Warning, TEXT("Item %d not found in player's inventory"), ItemID);
	return false;
#else
	return true;
#endif
}
