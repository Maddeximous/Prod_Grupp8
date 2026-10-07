// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FishTypes.h"
#include "Components/BoxComponent.h" 
#include "FishManager.generated.h"


UCLASS()
class PROD_GRUPP8_API AFishManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFishManager();
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Components")
	UBoxComponent* SpawnAreaBox;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FishData")
	TMap<EFishType, FFishData> FishDataBase;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FishData")
	TSubclassOf<class AFish> FishToSpawn;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FishData")
	int MaxAmountOfFish;
	
	UPROPERTY(EditAnywhere, Category = "FishData")
	TArray<AFish*> SpawnedFish;
	
	UFUNCTION(BlueprintCallable, Category = "Fishing")
	AFish* GetClosestFishToPlayer();
	
	void SpawnInitialFish();
	
	AFish* SpawnRandomFish();
	
	AFish* SpawnFishOfType(EFishType TypeToSpawn, FVector SpawnLocation);
	
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	void PlaySoundOnClosestFish(USoundBase* SoundToPlay);
	
	UFUNCTION(BlueprintCallable, Category = "Fishing")
	void OnFishCaught(AFish* CaughtFish);
	
private:
	FTimerHandle RespawnTimerHandle;

};
