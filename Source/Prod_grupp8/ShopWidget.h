// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShopWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROD_GRUPP8_API UShopWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(meta= (BindWidget))
	class UTextBlock* RodText;
	
	UFUNCTION(BlueprintCallable)
	void SetRodText(const FString& Text);
	
};
