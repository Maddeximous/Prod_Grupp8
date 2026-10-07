// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory.h"
#include "Fish.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AInventory::AInventory()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AInventory::BeginPlay()
{
	Super::BeginPlay();
	
	SpawnTestFish();
	
}

// Called every frame
void AInventory::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AInventory::AddFish(AFish* FishToAdd)
{
	if (FishToAdd)
	{
		Fish.Add(FishToAdd);
	}
}

void AInventory::RemoveFish(AFish* FishToRemove)
{
	if (FishToRemove)
	{
		Fish.Remove(FishToRemove);
	}
}

void AInventory::SpawnTestFish()
{
	for (int32 i = 0; i < 5; i++)
	{
		AFish* NewFish = GetWorld()->SpawnActor<AFish>(
			AFish::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator
		);

		if (NewFish)
		{
			// Pick a random fish species
			int32 RandomSpecies = FMath::RandRange(
				1,
				static_cast<int32>(EFishType::Sutare)
			);

			NewFish->FishSpecies =
				static_cast<EFishType>(RandomSpecies);

			// Random weight for testing
			NewFish->Weight = FMath::FRandRange(0.5f, 10.0f);

			AddFish(NewFish);
		}
	}
}


