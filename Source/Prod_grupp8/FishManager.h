// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FishTypes.h"
#include "FishManager.generated.h"

UCLASS()
class PROD_GRUPP8_API AFishManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFishManager();
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FishData")
	TMap<EFishType, FFishData> FishDataBase;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FishData")
	TSubclassOf<class AFish> FishToSpawn;
	
	UFUNCTION(BlueprintCallable, Category = "FishData")
	AFish* SpawnFishOfType(EFishType TypeToSpawn, FVector SpawnLocation);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
