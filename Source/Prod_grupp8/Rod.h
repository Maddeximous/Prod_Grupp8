// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <string>

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Rod.generated.h"

UCLASS()
class PROD_GRUPP8_API ARod : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARod();
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 price;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 durability;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 speed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 range;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 luck;
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
