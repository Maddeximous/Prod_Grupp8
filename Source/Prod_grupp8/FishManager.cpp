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
}

// Called every frame
void AFishManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	GetClosestFishToPlayer();
	
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
	if (FishDataBase.Num() == 0 || SpawnAreaBox == nullptr)
	{
		return nullptr;
	}
	
	//Fixa en random fish type från Enum. 
	TArray<EFishType> AvailableTypes;
	FishDataBase.GetKeys(AvailableTypes);
	EFishType RandomType = AvailableTypes[FMath::RandRange(0, AvailableTypes.Num() - 1)];
	
	//Hämta en random plats från SpawnAreaBox (det tillåtna området att fiska)
	FVector BoxOrigin = SpawnAreaBox->GetComponentLocation();
	FVector BoxExtent = SpawnAreaBox->GetScaledBoxExtent();
	FVector RandomLocation = FMath::RandPointInBox(FBox(BoxOrigin - BoxExtent, BoxOrigin + BoxExtent));
	RandomLocation.Z = BoxOrigin.Z;
	
	
	UE_LOG(LogTemp, Warning, TEXT("Random Fish: %s"), *RandomLocation.ToString());
	
	return SpawnFishOfType(RandomType, RandomLocation);
}

void AFishManager::OnFishCaught(AFish* CaughtFish)
{
	if (CaughtFish)
	{
		
		SpawnedFish.Remove(CaughtFish);

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
	
	//skapar en ny fisk. 
	FRotator SpawnRotation = FRotator::ZeroRotator;
	AFish* NewFish = GetWorld()->SpawnActor<AFish>(FishToSpawn, SpawnLocation, SpawnRotation);
	
	if (NewFish)
	{
		float RandomWeight = FMath::RandRange(FishInfo.MinWeight, FishInfo.MaxWeight);
		
		NewFish->FishSpecies = TypeToSpawn;
		NewFish->Weight = RandomWeight;
		UE_LOG(LogTemp, Warning, TEXT("Fish Type: %hdd, Fish Weight: %f"), NewFish->FishSpecies, NewFish->Weight);
	}
	
	SpawnedFish.Add(NewFish);
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

