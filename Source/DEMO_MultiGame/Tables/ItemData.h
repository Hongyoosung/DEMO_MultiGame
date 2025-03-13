#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemData.generated.h"

USTRUCT(BlueprintType) 
struct FItemData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FString ItemName;

	struct FItemFlags
	{
		uint64 Type : 8;        // 0~7비트
		uint64 Level : 4;       // 8~11비트
		uint64 Enhancement : 4; // 12~15비트
		uint64 Durability : 10; // 16~25비트: 내구도
		uint64 Option : 6;      // 26~31비트: 추가 옵션
		uint64 Reserved : 32;   // 32~63비트: 예비
	} ItemFlags;

	bool operator==(const FItemData& Other) const
	{
		return ItemID == Other.ItemID;
	}
};