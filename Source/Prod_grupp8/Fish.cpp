// Fill out your copyright notice in the Description page of Project Settings.


#include "Fish.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "src/Core/MathFunctions.h"
#include "UObject/ConstructorHelpers.h"
#include "Prod_grupp8Character.h"


// Sets default values
AFish::AFish()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	//Skapa en rooot component på objektet.
	USceneComponent* RootComp = CreateDefaultSubobject<USceneComponent>("RootComponent");
	RootComponent = RootComp;
	
	//Skapa en sphear på objektet och sätt fast den vid roten
	FishingSphere = CreateDefaultSubobject<USphereComponent>(TEXT("FishingSphere"));
	FishingSphere->SetupAttachment(RootComponent);
	
	FishingSphere->SetSphereRadius(300.0f);
	
	//Skapa ljud component
	FishingAudioComp = CreateDefaultSubobject<UAudioComponent>("FishingAudioComp");
	FishingAudioComp->SetupAttachment(RootComponent);
	FishingAudioComp->bAutoActivate = false;
	
	FishingSphere->SetCollisionProfileName("Trigger");
	
	bCanFish = false;
	
	static ConstructorHelpers::FObjectFinder<USoundBase> SoundAsset(TEXT("/Script/Engine.SoundWave'/Game/Audio/RawAudio/loop_bubbles_1.loop_bubbles_1'"));
	if (SoundAsset.Succeeded())
	{
		FishingSound = SoundAsset.Object;
	}

}

// Called when the game starts or when spawned
void AFish::BeginPlay()
{
	Super::BeginPlay();
	
	if (FishingSphere)
	{
		FishingSphere->OnComponentBeginOverlap.AddDynamic(this, &AFish::OnSphereOverlapBegin);
		FishingSphere->OnComponentEndOverlap.AddDynamic(this, &AFish::OnSphereOverlapEnd);
	}
	
	if (FishingAudioComp && FishingSound)
	{
		FishingAudioComp->SetSound(FishingSound);
	}
}

// Called every frame
void AFish::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AFish::OnSphereOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APawn* Pawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	
	if (OtherActor && OtherActor == Pawn)
	{
		if (AProd_grupp8Character* Player = Cast<AProd_grupp8Character>(Pawn))
		{
			Player->FishInRange = this;
		}
		bCanFish = true;
		UE_LOG(LogTemp, Warning, TEXT("Can Fish!"));
		
		if (FishingAudioComp && FishingSound)
		{
			if (!FishingAudioComp->IsPlaying())
			{
				UE_LOG(LogTemp, Warning, TEXT("Sound Playing!"));

				FishingAudioComp->Play();
			}
		}
	}
}

void AFish::OnSphereOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APawn* Pawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	
	if (OtherActor && OtherActor == Pawn)
	{
		if (AProd_grupp8Character* Player = Cast<AProd_grupp8Character>(Pawn))
		{
			Player->FishInRange = nullptr;
		}
		bCanFish = false;
		UE_LOG(LogTemp, Warning, TEXT("Can NOT Fish!"));
		
		if (FishingAudioComp && FishingAudioComp->IsPlaying())
		{
			FishingAudioComp->Stop();
		}
	}
}

