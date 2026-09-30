// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FishTypes.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EFishType : uint8
{
	None UMETA(DisplayName = "Ingen"),
	Aborre UMETA(DisplayName = "Aborre"),
	Gadda UMETA(DisplayName = "Gädda"),
	Gos UMETA(DisplayName = "Gös"),
	Insjooring UMETA(DisplayName = "Insjööring"),
	Roding UMETA(DisplayName = "Röding"),
	Regnbage UMETA(DisplayName = "Regnbåge"),
	Sik UMETA(DisplayName = "Sik"),
	Lake UMETA(DisplayName = "Lake"),
	Braxen UMETA(DisplayName = "Braxen"),
	Sutare UMETA(DisplayName = "Sutare"),
	
};

USTRUCT(BlueprintType)
struct FFishData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FishData")
	float MinWeight;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FishData")
	float MaxWeight;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FishData")
	float PricePerKilo;
	
};

class PROD_GRUPP8_API FishTypes
{
public:
	FishTypes();
	~FishTypes();
};
