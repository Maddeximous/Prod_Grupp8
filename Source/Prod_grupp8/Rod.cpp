// Fill out your copyright notice in the Description page of Project Settings.


#include "Rod.h"

// Sets default values
ARod::ARod()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARod::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARod::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

