// Fill out your copyright notice in the Description page of Project Settings.


#include "FishManager.h"
#include "Fish.h"

// Sets default values
AFishManager::AFishManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AFishManager::BeginPlay()
{
	Super::BeginPlay();
	
	SpawnInitialFish();
	
}

// Called every frame
void AFishManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AFishManager::SpawnInitialFish()
{
	for (int32 i = 0; i < 3; i++)
	{
		SpawnRandomFish();
	}
}

AFish* AFishManager::SpawnRandomFish()
{
	if (FishDataBase.Num() == 0 || SpawnLocations.Num() == 0)
	{
		return nullptr;
	}
	
	TArray<EFishType> AvailableTypes;
	FishDataBase.GetKeys(AvailableTypes);
	EFishType RandomType = AvailableTypes[FMath::RandRange(0, AvailableTypes.Num() - 1)];
	
	FVector RandomLocation = SpawnLocations[FMath::RandRange(0, SpawnLocations.Num() - 1)];
	
	UE_LOG(LogTemp, Warning, TEXT("Random Fish: %s"), *RandomLocation.ToString());
	
	return SpawnFishOfType(RandomType, RandomLocation);
}

void AFishManager::OnFishCaught(AFish* CaughtFish)
{
	if (CaughtFish)
	{
		CaughtFish->Destroy();
		
		float RespawnDelay = FMath::RandRange(20.0f, 30.0f);
		
		GetWorldTimerManager().SetTimer(
			RespawnTimerHandle,
			[this]() { SpawnRandomFish(); },
			RespawnDelay,
			false
			);
		
	}
}

AFish* AFishManager::SpawnFishOfType(EFishType TypeToSpawn, FVector SpawnLocation)
{
	if (!FishDataBase.Contains(TypeToSpawn))
	{
		return nullptr;
	}
	
	FFishData FishInfo = FishDataBase[TypeToSpawn];
	
	FRotator SpawnRotation = FRotator::ZeroRotator;
	AFish* NewFish = GetWorld()->SpawnActor<AFish>(FishToSpawn, SpawnLocation, SpawnRotation);
	
	if (NewFish)
	{
		float RandomWeight = FMath::RandRange(FishInfo.MinWeight, FishInfo.MaxWeight);
		
		NewFish->FishSpecies = TypeToSpawn;
		NewFish->Weight = RandomWeight;
		UE_LOG(LogTemp, Warning, TEXT("Fish Type: %hdd, Fish Weight: %f"), NewFish->FishSpecies, NewFish->Weight);
	}
	
	return NewFish;
}

