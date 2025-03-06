#include "AntiCheatManager.h"
#include "Characters/PlayerCharacter.h"
#include "Misc/Crc.h"

UAntiCheatManager* UAntiCheatManager::Instance = nullptr;

UAntiCheatManager::UAntiCheatManager()
{

}

UAntiCheatManager* UAntiCheatManager::GetInstance()
{
	if (!Instance)
	{
		Instance = NewObject<UAntiCheatManager>();
		Instance->AddToRoot();
	}

	return Instance;
}

bool UAntiCheatManager::VerifyAttackRange(const APlayerCharacter* Attacker, const APlayerCharacter* Target, const float MaxRange) const
{
	if (!Attacker || !Target) return false;

	const float Distance = FVector::Dist(Attacker->GetActorLocation(), Target->GetActorLocation());
	if (Distance > MaxRange)
	{
		UE_LOG(LogTemp, Warning, TEXT("Attack target is too far: %f units"), Distance);
		return false;
	}

	return true;
}

bool UAntiCheatManager::VerifyHealthChecksum(const APlayerCharacter* Player) const
{
	if (!Player)
	{
		UE_LOG(LogTemp, Warning, TEXT("VerifyHealthChecksum: Player is null"));
		return false;
	}

	// 플레이어의 HealthChecksum 값이 Health 필드의 현재 상태를 기반으로 계산된 CRC32 값과 일치하는지 확인
	// FCrc::MemCrc32는 메모리 블록의 CRC32 체크섬을 계산하는 함수
	const uint32 CalculatedChecksum = FCrc::MemCrc32(&Player->Health, sizeof(Player->Health));
	const uint32 StoredChecksum = Player->HealthChecksum;

	UE_LOG(LogTemp, Log, TEXT("VerifyHealthChecksum: Player=%s, Health=%.2f, CalculatedChecksum=%u, StoredChecksum=%u"),
		*Player->GetName(), Player->Health, CalculatedChecksum, StoredChecksum);

	bool bIsValid = (StoredChecksum == CalculatedChecksum);

	if (!bIsValid)
	{
		UE_LOG(LogTemp, Warning, TEXT("VerifyHealthChecksum: Checksum mismatch for Player %s"), *Player->GetName());
	}

	return bIsValid;
}

void UAntiCheatManager::UpdateHealthChecksum(APlayerCharacter* Player) const
{
	if (!Player) return;

	// 플레이어의 Health 필드 값을 기반으로 새로운 CRC32 체크섬을 계산하여 HealthChecksum에 저장
	Player->HealthChecksum = FCrc::MemCrc32(&Player->Health, sizeof(Player->Health));
}
