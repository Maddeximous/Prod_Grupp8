// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FishTypes.h"
#include "Fish.generated.h"

UCLASS()
class PROD_GRUPP8_API AFish : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFish();
	float Weight;
	EFishType FishSpecies;
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
