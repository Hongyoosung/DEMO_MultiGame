// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DEMO_MultiGameCharacter.h"
#include "PlayerCharacter.generated.h"

// Forward declarations
class AMultiGameMode;
class UAntiCheatManager;
class UHealthComponent;
class UPlayerUIComponent;
class UAntiCheatComponent;
class UInvenComponent;

struct FItemData;


UCLASS()
class DEMO_MULTIGAME_API APlayerCharacter : public ADEMO_MultiGameCharacter
{
    GENERATED_BODY()

public:
    APlayerCharacter();
    
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void Tick(float DeltaTime) override;

    
    // Take damage function
    UFUNCTION()
    void TakeDamage         (float Damage)          const;

    UFUNCTION()
    void TakeAcquireItem    (const FItemData& Item) const;

    UFUNCTION()
    void TakeUseItem        (int32 ItemID)          const;

    
    // Getter
    FORCEINLINE AMultiGameMode*         GetGameMode             ()      const   {       return GameMode;            }
    FORCEINLINE UAntiCheatComponent*    GetAntiCheatComponent   ()      const   {       return AntiCheatComponent;  }
    FORCEINLINE UHealthComponent*       GetHealthComponent      ()      const   {       return HealthComponent;     }
    FORCEINLINE UInvenComponent*        GetInvenComponent       ()      const   {       return InvenComponent;      }
    FORCEINLINE UPlayerUIComponent*     GetUIComponent          ()      const   {       return UIComponent;         }
    
    FORCEINLINE float                   GetAttackRange          ()      const   {       return AttackRange;         }

    TArray<FItemData> GetItemList() const;
    
    
protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

    
private:
    void InitializeManagers();

    
    // Task functions
    UFUNCTION()
    void Attack();

    UFUNCTION()
    void UseItem();
    
    UFUNCTION()
    void AcquireItem();

    
    // Verification methods
    bool AttackVerification (const APlayerCharacter* Player)                        const;
    bool ItemVerification   (const APlayerCharacter* Player, const int32 ItemID)    const;
    bool PlayerVerification (const APlayerCharacter* Player)                        const;

    
private:
    UPROPERTY()
    AMultiGameMode*        GameMode;
    
    UPROPERTY()
    UAntiCheatManager*      AntiCheatManager;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UHealthComponent*       HealthComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UInvenComponent*        InvenComponent;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UPlayerUIComponent*     UIComponent;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UAntiCheatComponent*    AntiCheatComponent;
    
    
    UPROPERTY()
    float                   AttackRange;
    

    // Getters for components - friend classes can use these
    friend class UHealthComponent;
    friend class UInvenComponent;
    friend class UPlayerUIComponent;
    friend class UAntiCheatComponent;
};