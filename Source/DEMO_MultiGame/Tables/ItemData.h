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
        uint64 Durability : 10; // 16~25비트
        uint64 Option : 6;      // 26~31비트
        uint64 Reserved : 32;   // 32~63비트

        FItemFlags() : Type(0), Level(0), Enhancement(0), Durability(0), Option(0), Reserved(0) {}

        // 전체 비트를 uint64로 변환
        uint64 ToUInt64() const
        {
            return (uint64)Type |
                   ((uint64)Level << 8) |
                   ((uint64)Enhancement << 12) |
                   ((uint64)Durability << 16) |
                   ((uint64)Option << 26) |
                   ((uint64)Reserved << 32);
        }

        // uint64에서 비트 필드로 복원
        void FromUInt64(uint64 Value)
        {
            Type = (uint8)(Value & 0xFF);
            Level = (uint8)((Value >> 8) & 0xF);
            Enhancement = (uint8)((Value >> 12) & 0xF);
            Durability = (uint16)((Value >> 16) & 0x3FF);
            Option = (uint8)((Value >> 26) & 0x3F);
            Reserved = (uint32)((Value >> 32) & 0xFFFFFFFF);
        }
    } ItemFlags;

    bool operator==(const FItemData& Other) const
    {
        return ItemID == Other.ItemID;
    }

    // 명시적 직렬화
    bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
    {
        Ar << ItemID;
        Ar << ItemName;

        if (Ar.IsSaving())
        {
            uint64 FlagsValue = ItemFlags.ToUInt64();
            Ar << FlagsValue;
        }
        else if (Ar.IsLoading())
        {
            uint64 FlagsValue;
            Ar << FlagsValue;
            ItemFlags.FromUInt64(FlagsValue);
        }

        bOutSuccess = true;
        return true;
    }
};

template<>
struct TStructOpsTypeTraits<FItemData> : public TStructOpsTypeTraitsBase2<FItemData>
{
    enum
    {
        WithNetSerializer = true,
    };
};