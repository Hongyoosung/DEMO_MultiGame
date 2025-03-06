#include "HealthBarWidget.h"
#include "Components/ProgressBar.h"


void UHealthBarWidget::NativeConstruct()
{
	UUserWidget::NativeConstruct();
}

void UHealthBarWidget::UpdateHealthBar(float HealthPercentage)
{
	if (HealthBar)
	{
		HealthBar->SetPercent(HealthPercentage);
	}
}

void UHealthBarWidget::UpdateHealthBarColor(bool bIsOwnPlayer) const
{
	if (HealthBar)
	{
		const FLinearColor BarColor = bIsOwnPlayer ? FLinearColor::Green : FLinearColor::Red;
		HealthBar->SetFillColorAndOpacity(BarColor);
	}
}
