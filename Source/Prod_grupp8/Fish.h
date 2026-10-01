// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FishTypes.h"
#include "Fish.generated.h"

class USphereComponent;
class UAudioComponent;
class USoundBase;

UCLASS()
class PROD_GRUPP8_API AFish : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFish();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish Properties")
	float Weight;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish Properties")
	EFishType FishSpecies;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* FishingSphere;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAudioComponent* FishingAudioComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fishing")
	bool bCanFish;
	
	UFUNCTION()
	void OnSphereOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* FishingSound;

};
