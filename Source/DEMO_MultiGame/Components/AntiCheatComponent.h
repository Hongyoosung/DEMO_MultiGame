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
	FORCEINLINE uint32		GetHealthChecksum		()	const					{		return HealthChecksum;			}
	FORCEINLINE uint32		GetPositionChecksum		()	const					{		return PositionChecksum;		}
	FORCEINLINE FVector		GetLastChecksumPosition	()	const					{		return LastChecksumPosition;	}
	
	// Setters
	FORCEINLINE void		SetHealthChecksum		(const uint32 Checksum)		{		HealthChecksum = Checksum;			}
	FORCEINLINE void		SetPositionChecksum		(const uint32 Checksum)		{		PositionChecksum = Checksum;		}
	FORCEINLINE void		SetLastChecksumPosition	(const FVector& Position)	{		LastChecksumPosition = Position;	}


private:
	UPROPERTY()
	uint32		HealthChecksum;

	UPROPERTY()
	uint32		PositionChecksum;

	UPROPERTY()
	FVector		LastChecksumPosition;
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DEMO_MULTIGAME_API UAntiCheatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAntiCheatComponent();

	void InitializeGameMode			(AMultiGameMode* InGameMode);
	
	// Verify checksum methods
	void UpdateAllChecksums			();
	bool ValidatePlayerForAction	() const;
	bool IsTargetInRange			(const APlayerCharacter* Target) const;

	
	FORCEINLINE FPlayerChecksum GetChecksums() const { return Checksums; }

	
protected:
	virtual void BeginPlay() override;

	
private:
	UPROPERTY(EditDefaultsOnly, Category = "AntiCheat")
	float ChecksumUpdateInterval;

	UPROPERTY()
	APlayerCharacter* OwnerCharacter;

	UPROPERTY()
	AMultiGameMode* GameMode;
	
	UPROPERTY()
	FPlayerChecksum Checksums;

	
	UAntiCheatManager* GetAntiCheatManager() const;
	float TimeSinceLastChecksumUpdate;
};