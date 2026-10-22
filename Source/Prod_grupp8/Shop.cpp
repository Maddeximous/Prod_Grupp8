
#include "Shop.h"
#include "ShopWidget.h"
#include "Inventory.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/InputComponent.h"
#include "InputCoreTypes.h"
#include "Prod_grupp8Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AShop::AShop()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox; // Sätt som Root så den syns i spelet

	TriggerBox->SetBoxExtent(FVector(150.0f, 150.0f, 100.0f)); // Storlek på boxen
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
}

// Called when the game starts or when spawned
void AShop::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetWorld()->GetFirstPlayerController();

	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AShop::OnBoxOverlapBegin);
		TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AShop::OnBoxOverlapEnd); // Avsluta shoppen när man går ut ur zonen
	}

	if (!PC)
		return;

	EnableInput(PC);

	if (!InputComponent)
		return;

	InputComponent->BindKey(EKeys::I, IE_Pressed, this, &AShop::ToggleShop);

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
	// Detta ser till att shoppen officiellt är stängd och skriver ut "Shop Closed" direkt när spelet startar
	CloseShop();
}
void AShop::OnBoxOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APawn* Pawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (OtherActor && OtherActor == Pawn)
	{
		if (AProd_grupp8Character* Player = Cast<AProd_grupp8Character>(Pawn))
		{
			Player->ShopInRange = this;
		}
		bCanOpenShop = true;
		UE_LOG(LogTemp, Warning, TEXT("Spelare gick in i shoppen!"));

		// Känn av att shoppen är stängd när man går in, och öppna den OMEDELBART
		if (!shopEnabled)
		{
			OpenShop(); // Detta sätter igång allt automatiskt!
		}
	}
}

void AShop::OnBoxOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APawn* Pawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    
	if (OtherActor && OtherActor == Pawn)
	{
		if (AProd_grupp8Character* Player = Cast<AProd_grupp8Character>(Pawn))
		{
			Player->ShopInRange = nullptr;
		}
		bCanOpenShop = false;

		// Stäng shoppen automatiskt när spelaren lämnar zonen
		if (shopEnabled)
		{
			CloseShop();
		}

		UE_LOG(LogTemp, Warning, TEXT("Spelare lämnade shoppen!"));
	}
}
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
				// Shoppen öppnas (valfritt om du vill göra musen synlig etc.)
			}
			else
			{
				// Shoppen stängs - ta bort DisableInput här!
			}
		}

		previousShopEnabled = shopEnabled;
	}
}

void AShop::addDollars()
{
	dollars += 100;

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
		}
	}
}
void AShop::OpenShop()
{
	shopEnabled = true;
	CurrentSection = EShopSection::Inventory;
	CurrentIndex = 0;

	APawn* Pawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (AProd_grupp8Character* Player = Cast<AProd_grupp8Character>(Pawn))
	{
		Player->GetCharacterMovement()->DisableMovement();
		// Stäng av fiske här om spelarklassen har en flagga för det, t.ex:
		// Player->bCanFish = false; 
	}

	GEngine->AddOnScreenDebugMessage(1, 10.0f, FColor::Blue, TEXT("Shop Opened"));
	PrintCurrentItem();
}

void AShop::CloseShop()
{
	shopEnabled = false;

	APawn* Pawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (AProd_grupp8Character* Player = Cast<AProd_grupp8Character>(Pawn))
	{
		Player->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		// Slå på fiske igen om det behövs
		// Player->bCanFish = true;
	}

	GEngine->AddOnScreenDebugMessage(1, 10.0f, FColor::Blue, TEXT("Shop Closed"));
}

void AShop::ToggleShop()
{
	// Om spelaren inte står i zonen ska man inte kunna öppna/stänga med knappen
	if (!bCanOpenShop)
	{
		return;
	}

	if (shopEnabled)
	{
		CloseShop();
	}
	else
	{
		OpenShop();
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
