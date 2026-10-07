// Fill out your copyright notice in the Description page of Project Settings.


#include "Shop.h"
#include "ShopWidget.h"
#include "Inventory.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/InputComponent.h"
#include "InputCoreTypes.h"
#include "SpeechSubsystem.h"

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
	
	
	APlayerController* PC = GetWorld()->GetFirstPlayerController();

	if (!PC)
		return;

	EnableInput(PC);
	

	if (!InputComponent)
		return;
	
	InputComponent->BindKey(
		EKeys::I,
		IE_Pressed,
		this,
		&AShop::ToggleShop
	);
	
	if (true)
	{
		InputComponent->BindKey(
			EKeys::T,
			IE_Pressed,
			this,
			&AShop::Transaction
		);
	
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
	
}

// Called every frame
void AShop::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (shopEnabled != previousShopEnabled)
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();

		if (PC)
		{
			if (shopEnabled)
			{
				EnableInput(PC);
			}
			else
			{
				DisableInput(PC);
			}
		}

		previousShopEnabled = shopEnabled;
	}

}

void AShop::addDollars()
{
	dollars+= 100;
	
	UE_LOG(LogTemp, Warning, TEXT("Dollars: %d"), dollars);
	GEngine->AddOnScreenDebugMessage(
		3,
		5.0f,
		FColor::Yellow,
		FString::Printf(
		TEXT("Money: %d"),
		dollars
		)
	);

}

void AShop::MoveLeft()
{
	if (shopEnabled)
	{
		CurrentSection = EShopSection::Inventory;
		CurrentIndex = 0;
	
		GEngine->AddOnScreenDebugMessage(
			3,
			5.0f,
			FColor::Yellow,
			FString::Printf(
				TEXT("Money: %d"),
				dollars
			)
		);
	}

	PrintCurrentItem();
}

void AShop::MoveRight()
{
	if (shopEnabled)
	{
		CurrentSection = EShopSection::Shop;
		CurrentIndex = 0;
		GEngine->AddOnScreenDebugMessage(
			3,
			5.0f,
			FColor::Yellow,
			FString::Printf(
				TEXT("Money: %d"),
				dollars
			)
		);

		PrintCurrentItem();
	}
}

void AShop::MoveUp()
{
	if (shopEnabled)
	{
		if (CurrentIndex > 0)
		{
			CurrentIndex--;
		}

		PrintCurrentItem();
	}
}
void AShop::MoveDown()
{
	if (shopEnabled)
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
}

void AShop::PrintCurrentItem()
{
	if (shopEnabled)
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
					1,
					10.0f,
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
					1,
					10.0f,
					FColor::Green,
					FString::Printf(
						TEXT("INVENTORY: %s | Weight: %.2f kg"),
						*FishTypeName,
						CurrentFish->Weight
					)
				);
				
				UGameInstance* GI = GetGameInstance();
				USpeechSubsystem* Speech = GI ? GI->GetSubsystem<USpeechSubsystem>() : nullptr;
				if (!Speech)
				{
					return;
				}

				// false = wait in line, don't cut off current speech
				Speech->Speak(
					FText::FromString(
					FString::Printf(
						TEXT("    : %s, : %.2f kilo"),
						*FishTypeName,
						CurrentFish->Weight
						)
						),
						false
				);
			}
		}
		else
		{
			if (Rods.Num() == 0)
			{
				GEngine->AddOnScreenDebugMessage(
					1,
					10.0f,
					FColor::Red,
					TEXT("Shop is empty")
				);

				return;
			}

			ARod* CurrentRod = Rods[CurrentIndex];

			if (CurrentRod)
			{
				GEngine->AddOnScreenDebugMessage(
					1,
					10.0f,
					FColor::Green,
					FString::Printf(
					TEXT("Shop: %s | price: %d | luck: %d"),
					*CurrentRod->name.ToString(),
					CurrentRod->price,
					CurrentRod->luck
					)
				);
			}
			
			UGameInstance* GI = GetGameInstance();
			USpeechSubsystem* Speech = GI ? GI->GetSubsystem<USpeechSubsystem>() : nullptr;
			if (!Speech)
			{
				return;
			}

			// false = wait in line, don't cut off current speech
			Speech->Speak(
				FText::FromString(
				FString::Printf(
				TEXT("Fiskespö: %s | pris: %d | kronor kvalitet: %d"),
				*CurrentRod->name.ToString(),
				CurrentRod->price,
				CurrentRod->luck
					)
					),
					false
			);
		}
	}
}

void AShop::ToggleShop()
{
	AShop::shopEnabled = !AShop::shopEnabled;
	
	CurrentSection = EShopSection::Inventory;
	CurrentIndex = 0;
	
	if (shopEnabled)
	{
		GEngine->AddOnScreenDebugMessage(
			1,
			10.0f,
			FColor::Blue,
			TEXT("Shop Opened")
		);
		UGameInstance* GI = GetGameInstance();
		USpeechSubsystem* Speech = GI ? GI->GetSubsystem<USpeechSubsystem>() : nullptr;
		if (!Speech)
		{
			return;
		}

		// false = wait in line, don't cut off current speech
		Speech->Speak(
			FText::FromString(
			FString::Printf(
			TEXT("Butiken öppnad")
			
				)
				),
				false
		);
	}
	if (!shopEnabled)
	{
		GEngine->AddOnScreenDebugMessage(
			1,
			10.0f,
			FColor::Blue,
			TEXT("Shop Closed")
		);
		UGameInstance* GI = GetGameInstance();
		USpeechSubsystem* Speech = GI ? GI->GetSubsystem<USpeechSubsystem>() : nullptr;
		if (!Speech)
		{
			return;
		}

		// false = wait in line, don't cut off current speech
		Speech->Speak(
			FText::FromString(
			FString::Printf(
			TEXT("Butiken stängd")
			
				)
				),
				false
		);
	}
}

void AShop::Transaction()
{
	if (!PlayerInventory)
	{
		return;
	}

	if (CurrentSection == EShopSection::Inventory)
	{
		if (PlayerInventory->Fish.Num() == 0)
		{
			return;
		}

		AFish* CurrentFish =
			PlayerInventory->Fish[CurrentIndex];

		if (!CurrentFish)
		{
			return;
		}

		int32 SellPrice = FMath::RoundToInt(CurrentFish->Weight);
		

		dollars += SellPrice;

		PlayerInventory->RemoveFish(CurrentFish);
		CurrentFish->Destroy();

		GEngine->AddOnScreenDebugMessage(
			2,
			3.0f,
			FColor::Green,
			FString::Printf(
				TEXT("SOLD FISH FOR %d DOLLARS"),
				SellPrice
			)
		);
		UGameInstance* GI = GetGameInstance();
		USpeechSubsystem* Speech = GI ? GI->GetSubsystem<USpeechSubsystem>() : nullptr;
		if (!Speech)
		{
			return;
		}

		// false = wait in line, don't cut off current speech
		Speech->Speak(
			FText::FromString(
			FString::Printf(
			TEXT("Sålde fisk för %d kronor"),
				SellPrice
				)
				),
				false
		);
		Speech->Speak(
			FText::FromString(
			FString::Printf(
			TEXT("Du har: %d kronor"),
				dollars
				)
				),
				false
		);
		GEngine->AddOnScreenDebugMessage(
			3,
			5.0f,
			FColor::Yellow,
			FString::Printf(
				TEXT("Money: %d"),
				dollars
			)
		);

		// Make sure the index is still valid
		if (CurrentIndex >= PlayerInventory->Fish.Num())
		{
			CurrentIndex =
				FMath::Max(0, PlayerInventory->Fish.Num() - 1);
		}

		PrintCurrentItem();
	}
	else
	{
		if (Rods.Num() == 0)
		{
			return;
		}

		ARod* CurrentRod = Rods[CurrentIndex];

		if (!CurrentRod)
		{
			return;
		}

		int32 RodPrice = CurrentRod->price;

		if (dollars < RodPrice)
		{
			GEngine->AddOnScreenDebugMessage(
				2,
				3.0f,
				FColor::Red,
				TEXT("NOT ENOUGH MONEY")
			);
			UGameInstance* GI = GetGameInstance();
			USpeechSubsystem* Speech = GI ? GI->GetSubsystem<USpeechSubsystem>() : nullptr;
			if (!Speech)
			{
				return;
			}

			// false = wait in line, don't cut off current speech
			Speech->Speak(
				FText::FromString(
				FString::Printf(
				TEXT("För lite pengar")
				
					)
					),
						false
			);
			return;
		}
		
		

		ARod* NewRod = GetWorld()->SpawnActor<ARod>(
			ARod::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator
		);

		if (!NewRod)
		{
			return;
		}

		NewRod->name = CurrentRod->name;
		NewRod->price = CurrentRod->price;
		NewRod->luck = CurrentRod->luck;

		dollars -= RodPrice;
		PlayerInventory->Rods.Add(NewRod);

		GEngine->AddOnScreenDebugMessage(
			2,
			3.0f,
			FColor::Green,
			FString::Printf(
				TEXT("BOUGHT %s FOR %d DOLLARS"),
				*CurrentRod->name.ToString(),
				RodPrice
			)
		);
		
		UGameInstance* GI = GetGameInstance();
		USpeechSubsystem* Speech = GI ? GI->GetSubsystem<USpeechSubsystem>() : nullptr;
		if (!Speech)
		{
			return;
		}

		// false = wait in line, don't cut off current speech
		Speech->Speak(
			FText::FromString(
			FString::Printf(
			TEXT("Köpte %s för %d kronor"),
				*CurrentRod->name.ToString(),
				RodPrice
				)
				),
				false
		);
		Speech->Speak(
			FText::FromString(
			FString::Printf(
			TEXT("Du har: %d kronor"),
				dollars
				)
				),
				false
		);
		GEngine->AddOnScreenDebugMessage(
			3,
			5.0f,
			FColor::Yellow,
			FString::Printf(
				TEXT("Money: %d"),
				dollars
			)
		);

		PrintCurrentItem();
	}

}
