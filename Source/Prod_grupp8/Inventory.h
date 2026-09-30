// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Fish.h"
#include "Inventory.generated.h"

UCLASS()
class PROD_GRUPP8_API AInventory : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInventory();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<AFish*> Fish;
	
	UFUNCTION(BlueprintCallable)
	void AddFish(AFish* FishToAdd);

	UFUNCTION(BlueprintCallable)
	void RemoveFish(AFish* FishToRemove);


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
