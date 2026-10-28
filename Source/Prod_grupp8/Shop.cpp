#include "Shop.h"
#include "ShopWidget.h"
#include "Inventory.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/InputComponent.h"
#include "InputCoreTypes.h"
#include "Prod_grupp8Character.h"
#include "GameFramework/CharacterMovementComponent.h"

AShop::AShop()
{
	// Nothing in the shop needs to tick anymore
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;

	TriggerBox->SetBoxExtent(FVector(150.0f, 150.0f, 100.0f));
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));

	ShopAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ShopAudioComponent"));
	ShopAudioComponent->SetupAttachment(RootComponent);
	ShopAudioComponent->bAutoActivate = true;
}

void AShop::BeginPlay()
{
	Super::BeginPlay();
	
	FishManager = Cast<AFishManager>(UGameplayStatics::GetActorOfClass(this, AFishManager::StaticClass()));
	
	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AShop::OnBoxOverlapBegin);
		TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AShop::OnBoxOverlapEnd);
	}

	if (ShopWidgetClass)
	{
		ShopWidget = CreateWidget<UShopWidget>(GetWorld(), ShopWidgetClass);
		if (ShopWidget)
		{
			ShopWidget->AddToViewport();
		}
	}
	

	RefreshWidget();

	// Makes sure the shop is officially closed (and prints "Shop Closed") when the game starts
	CloseShop();
}

void AShop::OnBoxOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
	APawn* Pawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (OtherActor && OtherActor == Pawn)
	{
		if (AProd_grupp8Character* Player = Cast<AProd_grupp8Character>(Pawn))
		{
			Player->OnShopEnetered();
			Player->ShopInRange = this;
		}
		bCanOpenShop = true;
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

		// Close the shop automatically when the player leaves the zone
		if (shopEnabled)
		{
			CloseShop();
		}

		UE_LOG(LogTemp, Warning, TEXT("Spelare lämnade shoppen!"));
	}
}

void AShop::OpenShop()
{
	shopEnabled = true;

	APawn* Pawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (AProd_grupp8Character* Player = Cast<AProd_grupp8Character>(Pawn))
	{
		Player->OnShopOpened();
		Player->GetCharacterMovement()->DisableMovement();
	}

	ShowMessage(1, 10.0f, FColor::Blue, TEXT("Shop Opened | Down: sell all fish | Up: buy next rod"));
	OnEnterShop();
	ShowMoney();
	RefreshWidget();
}

void AShop::CloseShop()
{
	shopEnabled = false;

	APawn* Pawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (AProd_grupp8Character* Player = Cast<AProd_grupp8Character>(Pawn))
	{
		Player->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}

	ShowMessage(1, 10.0f, FColor::Blue, TEXT("Shop Closed"));
}

void AShop::ToggleShop()
{
	// Can only open/close while standing in the zone
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

void AShop::SellAllFish()
{
	if (!shopEnabled || !PlayerInventory)
	{
		return;
	}

	// Work on a copy: RemoveFish modifies the inventory's array while we loop over it
	const auto FishToSell = PlayerInventory->Fish;

	int32 FishSold = 0;
	int32 TotalPrice = 0;

	for (AFish* Fish : FishToSell)
	{
		if (!IsValid(Fish))
		{
			continue;
		}
		
		TotalPrice += FishManager -> FishDataBase[Fish -> FishSpecies].PricePerKilo * Fish->Weight;
		PlayerInventory->RemoveFish(Fish);
		Fish->Destroy();
		++FishSold;
	}

	if (FishSold == 0)
	{
		// PlayNoMoneySoundWithDelay(1.0f);
		ShowMessage(2, 3.0f, FColor::Red, TEXT("NO FISH TO SELL"));
		return;
	}

	UGameplayStatics::PlaySound2D(this, SellSound, 1.0f, 1.0f, 0.0f);
	dollars += TotalPrice;

	ShowMessage(2, 3.0f, FColor::Green, FString::Printf(TEXT("SOLD %d FISH FOR %d DOLLARS"), FishSold, TotalPrice));
	OnFishSold(FishSold, TotalPrice);
	ShowMoney();
}

void AShop::BuyNextRod()
{
	if (!shopEnabled || !PlayerInventory)
	{
		return;
	}

	if (!Rods.IsValidIndex(NextRodIndex))
	{
		PlayNoMoneySoundWithDelay(1.0f);
		ShowMessage(2, 3.0f, FColor::Yellow, TEXT("ALL RODS OWNED"));
		return;
	}

	ARod* RodForSale = Rods[NextRodIndex];
	if (!RodForSale)
	{
		PlayNoMoneySoundWithDelay(1.0f);
		UE_LOG(LogTemp, Warning, TEXT("Shop: Rods[%d] is null"), NextRodIndex);
		return;
	}

	const int32 RodPrice = RodForSale->price;
	if (dollars < RodPrice)
	{
		PlayNoMoneySoundWithDelay(1.0f);
		ShowMessage(2, 3.0f, FColor::Red, FString::Printf(TEXT("NOT ENOUGH MONEY (%d needed)"), RodPrice));
		return;
	}

	// Spawn the same class as the shop rod so Blueprint rods keep their class
	ARod* NewRod = GetWorld()->SpawnActor<ARod>(
		RodForSale->GetClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator
	);

	if (!NewRod)
	{
		return;
	}

	NewRod->name = RodForSale->name;
	NewRod->price = RodForSale->price;
	NewRod->luck = RodForSale->luck;
	NewRod->RodTier = RodForSale->RodTier;

	UGameplayStatics::PlaySound2D(this, BuySound, 1.0f, 1.0f, 0.0f);
	dollars -= RodPrice;
	PlayerInventory->Rods.Add(NewRod);
	PlayerInventory->EquipRod(NewRod);

	++NextRodIndex;
	ShowMessage(2, 3.0f, FColor::Green, FString::Printf(TEXT("BOUGHT %s FOR %d DOLLARS"), *RodForSale->name.ToString(), RodPrice));
	//OnRodBought(RodForSale->name, RodPrice);
	ShowMoney();
	RefreshWidget();
}

void AShop::addDollars()
{
	dollars += 100;

	UE_LOG(LogTemp, Warning, TEXT("Dollars: %d"), dollars);
	ShowMoney();
}

void AShop::Transaction()
{
	// Replaced by SellAllFish / BuyNextRod
}

void AShop::ToggleSection()
{
	// No sections anymore
}

void AShop::RefreshWidget()
{
	if (!ShopWidget)
	{
		return;
	}

	FString Text;

	if (Rods.IsValidIndex(NextRodIndex) && Rods[NextRodIndex])
	{
		const ARod* Rod = Rods[NextRodIndex];
		Text = FString::Printf(
			TEXT("Next rod: %s | price: %d | luck: %d"),
			*Rod->name.ToString(),
			Rod->price,
			Rod->luck
		);
	}
	else
	{
		Text = TEXT("All rods owned");
	}

	ShopWidget->SetRodText(Text);
}

void AShop::ShowMoney() const
{
	ShowMessage(3, 5.0f, FColor::Yellow, FString::Printf(TEXT("Money: %d"), dollars));
}

void AShop::ShowMessage(int32 Key, float Time, const FColor& Color, const FString& Text) const
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(Key, Time, Color, Text);
	}
}

void AShop::PlayNoMoneySoundWithDelay(float DelaySeconds)
{
	if (!NoMoneySound)
	{
		return;
	}

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		[this]()
		{
			UGameplayStatics::PlaySound2D(this, NoMoneySound, 1.0f, 1.0f, 0.0f);
		},
		DelaySeconds,
		false
	);
}