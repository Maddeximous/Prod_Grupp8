// Fill out your copyright notice in the Description page of Project Settings.


#include "Shop.h"
#include "ShopWidget.h"
#include "Inventory.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
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
	InputComponent->BindKey(
		EKeys::Left,
		IE_Pressed,
		this,
		&AShop::MoveLeft
	);

	InputComponent->BindKey(
		EKeys::Right,
		IE_Pressed,
		this,
		&AShop::MoveRight
	);

	InputComponent->BindKey(
		EKeys::Up,
		IE_Pressed,
		this,
		&AShop::MoveUp
	);

	InputComponent->BindKey(
		EKeys::Down,
		IE_Pressed,
		this,
		&AShop::MoveDown
	);

	PrintCurrentItem();

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

void AShop::MoveLeft()
{
	CurrentSection = EShopSection::Inventory;
	CurrentIndex = 0;

	PrintCurrentItem();
}

void AShop::MoveRight()
{
	CurrentSection = EShopSection::Shop;
	CurrentIndex = 0;

	PrintCurrentItem();
}

void AShop::MoveUp()
{
	if (CurrentIndex > 0)
	{
		CurrentIndex--;
	}

	PrintCurrentItem();
}
void AShop::MoveDown()
{
	int32 ItemCount = 0;

	if (CurrentSection == EShopSection::Inventory)
	{
		if (PlayerInventory)
		{
			ItemCount = PlayerInventory->Fish.Num();
		}
	}
	else
	{
		ItemCount = Rods.Num();
	}

	if (CurrentIndex < ItemCount - 1)
	{
		CurrentIndex++;
	}

	PrintCurrentItem();
}

void AShop::PrintCurrentItem()
{
	if (!GEngine)
	{
		return;
	}

	if (CurrentSection == EShopSection::Inventory)
	{
		if (!PlayerInventory || PlayerInventory->Fish.Num() == 0)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				2.0f,
				FColor::Red,
				TEXT("Inventory is empty")
			);

			return;
		}

		AFish* CurrentFish = PlayerInventory->Fish[CurrentIndex];

		if (CurrentFish)
		{
			FString FishTypeName;

			switch (CurrentFish->FishSpecies)
			{
			case EFishType::Aborre:
				FishTypeName = TEXT("Aborre");
				break;

			case EFishType::Gadda:
				FishTypeName = TEXT("Gädda");
				break;

			case EFishType::Gos:
				FishTypeName = TEXT("Gös");
				break;

			case EFishType::Insjooring:
				FishTypeName = TEXT("Insjööring");
				break;

			case EFishType::Roding:
				FishTypeName = TEXT("Röding");
				break;

			case EFishType::Regnbage:
				FishTypeName = TEXT("Regnbåge");
				break;

			case EFishType::Sik:
				FishTypeName = TEXT("Sik");
				break;

			case EFishType::Lake:
				FishTypeName = TEXT("Lake");
				break;

			case EFishType::Braxen:
				FishTypeName = TEXT("Braxen");
				break;

			case EFishType::Sutare:
				FishTypeName = TEXT("Sutare");
				break;

			default:
				FishTypeName = TEXT("Unknown");
				break;
			}

			GEngine->AddOnScreenDebugMessage(
				-1,
				2.0f,
				FColor::Green,
				FString::Printf(
					TEXT("INVENTORY: %s | Weight: %.2f kg"),
					*FishTypeName,
					CurrentFish->Weight
				)
			);
		}
	}
	else
	{
		if (Rods.Num() == 0)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				2.0f,
				FColor::Red,
				TEXT("Shop is empty")
			);

			return;
		}

		ARod* CurrentRod = Rods[CurrentIndex];

		if (CurrentRod)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				2.0f,
				FColor::Green,
				FString::Printf(
				TEXT("Rod: %s | price: %d | luck: %d"),
				*CurrentRod->name.ToString(),
				CurrentRod->price,
				CurrentRod->luck
				)
			);
		}
	}
}
