// Fill out your copyright notice in the Description page of Project Settings.


#include "FishManager.h"
#include "Fish.h"
#include "Kismet/GameplayStatics.h"
#include "Components/ArrowComponent.h"

// Sets default values
AFishManager::AFishManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	SpawnAreaBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnAreaBox"));
	RootComponent = SpawnAreaBox;

}

// Called when the game starts or when spawned
void AFishManager::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Error, TEXT("FISH MANAGER BEGINPLAY: %s"), *GetName());
	
	Inventory = Cast<AInventory>(
		UGameplayStatics::GetActorOfClass(this, AInventory::StaticClass()));

	if (!IsValid(Inventory))
	{
		UE_LOG(LogTemp, Error,
			TEXT("FishManager: kunde inte hitta någon AInventory i nivån."));
		return;
	}

	// Om inventoryt redan har spön, använd det första som startspö.
	if (!Inventory->EquippedRod && Inventory->Rods.Num() > 0)
	{
		Inventory->EquipRod(Inventory->Rods[0]);
	}

	if (!Inventory->EquippedRod)
	{
		UE_LOG(LogTemp, Error,
			TEXT("FishManager: inget EquippedRod. Lägg till ett startspö i Inventory."));
		return;
	}

	UE_LOG(LogTemp, Warning,
		TEXT("FishManager: använder spö %s"),
		*Inventory->EquippedRod->name.ToString());

	SpawnInitialFish();
}

// Called every frame
void AFishManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	GetClosestFishToPlayer();
	
}

void AFishManager::SpawnInitialFish()
{
	int AmountOfFish = FMath::RandRange(3, 5);
	
	for (int32 i = 0; i < AmountOfFish; i++)
	{
		SpawnRandomFish();
		UE_LOG(LogTemp, Warning, TEXT("We have spawned fish!"));
	}
}

AFish* AFishManager::SpawnRandomFish()
{
	if (FishDataBase.Num() == 0 || !SpawnAreaBox)
	{
		return nullptr;
	}

	TArray<EFishType> AvailableTypes;

	for (const auto& Entry : FishDataBase)
	{
		if (CanSpawnFishOfType(Entry.Key))
		{
			AvailableTypes.Add(Entry.Key);
		}
	}

	if (AvailableTypes.Num() == 0)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("No fish are available for the equipped rod."));
		return nullptr;
	}

	const EFishType RandomType =
		AvailableTypes[FMath::RandRange(0, AvailableTypes.Num() - 1)];

	const FVector BoxOrigin = SpawnAreaBox->GetComponentLocation();
	const FVector BoxExtent = SpawnAreaBox->GetScaledBoxExtent();

	FVector RandomLocation = FMath::RandPointInBox(
		FBox(BoxOrigin - BoxExtent, BoxOrigin + BoxExtent));

	RandomLocation.Z = BoxOrigin.Z;

	return SpawnFishOfType(RandomType, RandomLocation);
}

bool AFishManager::CanSpawnFishOfType(EFishType FishType) const
{
	const FFishData* FishInfo = FishDataBase.Find(FishType);

	if (!FishInfo || !Inventory || !Inventory->EquippedRod)
	{
		return false;
	}

	return static_cast<uint8>(Inventory->EquippedRod->RodTier) >=
		   static_cast<uint8>(FishInfo->RequiredRodTier);
}

void AFishManager::OnFishCaught(AFish* CaughtFish)
{
	if (CaughtFish)
	{
		
		SpawnedFish.Remove(CaughtFish);
		
		Inventory -> AddFish(CaughtFish);
		CaughtFish -> SetActorLocation(FVector(0,0, -10000.0));
		
		float RespawnDelay = FMath::RandRange(5.0f, 10.0f);
		
		int AmountOfFishToSpawn = FMath::RandRange(1, 3);
		
		if (SpawnedFish.Num() == MaxAmountOfFish) AmountOfFishToSpawn = 0;
		
		for (int32 i = 0; i < AmountOfFishToSpawn; i++)
		{
			GetWorldTimerManager().SetTimer(
			RespawnTimerHandle,
			[this]() { SpawnRandomFish(); },
			RespawnDelay,
			false
			);
		}
	}
}

AFish* AFishManager::SpawnFishOfType(
	EFishType TypeToSpawn,
	FVector SpawnLocation)
{
	if (!CanSpawnFishOfType(TypeToSpawn))
	{
		return nullptr;
	}

	const FFishData* FishInfo = FishDataBase.Find(TypeToSpawn);
	if (!FishInfo)
	{
		return nullptr;
	}

	AFish* NewFish = GetWorld()->SpawnActor<AFish>(
		FishToSpawn,
		SpawnLocation,
		FRotator::ZeroRotator);

	if (NewFish)
	{
		NewFish->FishSpecies = TypeToSpawn;
		NewFish->Weight =
			FMath::RandRange(FishInfo->MinWeight, FishInfo->MaxWeight);

		SpawnedFish.Add(NewFish);
	}

	return NewFish;
}

AFish* AFishManager::GetClosestFishToPlayer()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	
	if (!PlayerPawn || SpawnedFish.Num() == 0)
	{
		return nullptr;
	}
	
	FVector PlayerLocation = PlayerPawn->GetActorLocation();
	AFish* ClosestFish = nullptr;
	
	float ClosestDistanceSq = FLT_MAX;
	
	for (AFish* Fish : SpawnedFish)
	{
		if (IsValid(Fish))
		{
			float DistanceSq = FVector::DistSquared(PlayerLocation, Fish->GetActorLocation());
			
			if (DistanceSq < ClosestDistanceSq)
			{
				ClosestDistanceSq = DistanceSq;
				ClosestFish = Fish;
			}
		}
	}
	
	return ClosestFish;
}

void AFishManager::PlaySoundOnClosestFish(USoundBase* SoundToPlay)
{
	AFish* closestFish = GetClosestFishToPlayer();
	UGameplayStatics::SpawnSoundAtLocation(this, SoundToPlay, closestFish->GetActorLocation(), closestFish->GetActorRotation());
}
