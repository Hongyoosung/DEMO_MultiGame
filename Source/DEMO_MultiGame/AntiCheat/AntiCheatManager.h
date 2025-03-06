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
	static UAntiCheatManager* GetInstance();

	bool VerifyAttackRange(const APlayerCharacter* Attacker, const APlayerCharacter* Target, const float MaxRange) const;
	bool VerifyHealthChecksum(const APlayerCharacter* Player) const;
	void UpdateHealthChecksum(APlayerCharacter* Player) const;
	
private:
	UAntiCheatManager();
	static UAntiCheatManager* Instance;
};
