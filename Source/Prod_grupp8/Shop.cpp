// Fill out your copyright notice in the Description page of Project Settings.


#include "Shop.h"
#include "ShopWidget.h"
#include "Components/InputComponent.h"
#include "InputCoreTypes.h"

// Sets default values
AShop::AShop()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AShop::BeginPlay()
{
	Super::BeginPlay();
	
	EnableInput(GetWorld()->GetFirstPlayerController());

	InputComponent->BindKey(
		EKeys::E,
		IE_Pressed,
		this,
		&AShop::addDollars
	);

	for (ARod* Rod : Rods)
	{
		if (Rod)
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("Rod: %s | price: %d | luck: %d"),
				*Rod->name.ToString(),
				Rod->price,
				Rod->luck
			);
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					5.0f,
					FColor::Green,
					FString::Printf(
					TEXT("Rod: %s | price: %d | luck: %d"),
					*Rod->name.ToString(),
					Rod->price,
					Rod->luck
					)
				);
			}
		}
	}
	
	if (ShopWidgetClass)
	{
		UShopWidget* ShopWidget = CreateWidget<UShopWidget>(
			GetWorld(),
			ShopWidgetClass
		);
		if (ShopWidget)
		{
			FString ShopText;

			for (ARod* Rod : Rods)
			{
				if (Rod)
				{
					ShopText += FString::Printf(
					TEXT("Rod: %s | price: %d | luck: %d"),
					*Rod->name.ToString(),
					Rod->price,
					Rod->luck
					);
				}
			}

			ShopWidget->SetRodText(ShopText);
			ShopWidget->AddToViewport();
		}
	}
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,                         // Message key
			5.0f,                       // Display time
			FColor::Green,              // Color
			TEXT("WELCOME TO THE SHOP!")
		);
	}
	
}

// Called every frame
void AShop::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AShop::addDollars()
{
	dollars+= 100;
	
	UE_LOG(LogTemp, Warning, TEXT("Gold: %d"), dollars);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			2.0f,
			FColor::Green,
			FString::Printf(TEXT("dollars: %d"), dollars)
		);
	}
}

