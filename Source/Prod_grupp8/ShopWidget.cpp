// Fill out your copyright notice in the Description page of Project Settings.


#include "ShopWidget.h"
#include "Components/TextBlock.h"

void UShopWidget::SetRodText(const FString& Text)
{
	if (RodText)
	{
		RodText->SetText(FText::FromString(Text));
	}
}