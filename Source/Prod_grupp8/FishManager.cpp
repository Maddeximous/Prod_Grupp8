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
	
}

// Called every frame
void AFishManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
	}
	
	return NewFish;
}

