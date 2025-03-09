#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBarWidget.generated.h"

class UProgressBar;

UCLASS()
class UHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 위젯이 초기화될 때 호출
	virtual void NativeConstruct() override;
	
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateHealthBar(float HealthPercentage);
	
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateHealthBarColor(bool bIsOwnPlayer) const;
	
protected:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;
};
