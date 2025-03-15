#include "HealthBarWidget.h"
#include "Components/ProgressBar.h"


void UHealthBarWidget::NativeConstruct()
{
//#ifndef UE_SERVER
	UUserWidget::NativeConstruct();
//#endif
}

void UHealthBarWidget::UpdateHealthBar(float HealthPercentage)
{
//#ifndef UE_SERVER
	if (HealthBar)
	{
		HealthBar->SetPercent(HealthPercentage);
	}
//#endif
}

void UHealthBarWidget::UpdateHealthBarColor(bool bIsOwnPlayer) const
{
//#ifndef UE_SERVER
	if (HealthBar)
	{
		const FLinearColor BarColor = bIsOwnPlayer ? FLinearColor::Green : FLinearColor::Red;
		HealthBar->SetFillColorAndOpacity(BarColor);
	}
//#endif
}
