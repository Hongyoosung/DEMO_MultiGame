// UI component to manage player interface elements

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/WidgetComponent.h"
#include "PlayerUIComponent.generated.h"


class UHealthBarWidget;
class APlayerCharacter;


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DEMO_MULTIGAME_API UPlayerUIComponent : public UActorComponent
{
	GENERATED_BODY()

	
public:
	UPlayerUIComponent();
    
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	
	void InitializeHealthWidget();

	
	UFUNCTION()
	void UpdateHealthUI(float HealthPercent);

	
protected:
	virtual void BeginPlay() override;

	
private:
	// UI Widget references
	UPROPERTY(VisibleAnywhere, Category = "UI")
	UWidgetComponent* HealthBarWidgetComponent;
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	UHealthBarWidget* HealthBarWidget;
	
	// Widget classes
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> HealthWidgetClass;
	
	// Owner reference
	UPROPERTY()
	APlayerCharacter* OwnerCharacter;
};