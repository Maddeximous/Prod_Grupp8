// Copyright Epic Games, Inc. All Rights Reserved.

#include "Prod_grupp8Character.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "FishManager.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Prod_grupp8.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AProd_grupp8Character::AProd_grupp8Character()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	
	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;
}

void AProd_grupp8Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		//EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AProd_grupp8Character::DoJumpStart);
		//EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AProd_grupp8Character::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AProd_grupp8Character::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AProd_grupp8Character::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AProd_grupp8Character::LookInput);
		
		// Casting fishing line
		EnhancedInputComponent->BindAction(CastLineAction, ETriggerEvent::Started, this, &AProd_grupp8Character::DoCastLine);
		
		// Reeling fishing line
		EnhancedInputComponent->BindAction(ReelLineAction, ETriggerEvent::Triggered, this, &AProd_grupp8Character::ReelLineInput);
		
		// Sonar
		EnhancedInputComponent->BindAction(SonarAction, ETriggerEvent::Started, this, &AProd_grupp8Character::DoSonar);
		
		// Sway right
		EnhancedInputComponent->BindAction(SwayRightAction, ETriggerEvent::Started, this, &AProd_grupp8Character::DoSwayRight);
		
		// Sway left
		EnhancedInputComponent->BindAction(SwayLeftAction, ETriggerEvent::Started, this, &AProd_grupp8Character::DoSwayLeft);
	}
	else
	{
		UE_LOG(LogProd_grupp8, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}
void AProd_grupp8Character::BeginPlay()
{
	Super::BeginPlay();

	FishManager = Cast<AFishManager>(UGameplayStatics::GetActorOfClass(this, AFishManager::StaticClass()));

	if (ReelInSoundCue)
	{
		ReelInSoundCueObject = UGameplayStatics::SpawnSound2D(this, ReelInSoundCue, 1.0f, 1.0f, 0.0f);
		
		if (ReelInSoundCueObject)
		{
			ReelInSoundCueObject->Stop();
		}
	}
}

void AProd_grupp8Character::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(0, MovementVector.Y);

}
void AProd_grupp8Character::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);

}


void AProd_grupp8Character::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		if (!bIsFishing)
		{
			// pass the move inputs
			AddMovementInput(GetActorRightVector(), Right);
			AddMovementInput(GetActorForwardVector(), Forward);
		}
	}
}



void AProd_grupp8Character::DoJumpStart()
{
	// pass Jump to the character
	Jump();
}

void AProd_grupp8Character::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();
}
void AProd_grupp8Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bIsFishOnHook)
	{
		if (bFishIsFighting)
		{
			FishFightTimer += DeltaTime;
			if (FishFightTimer > 4)
			{
				bFishIsFighting = false;
			}
			
			if (FMath::RandBool())
			{
				FishFightingLeft(DeltaTime);
			}
			else
			{
				FishFightingRight(DeltaTime);
			}
		}
		else
		{
			Fishing(DeltaTime);
		}
	}
}

void AProd_grupp8Character::DoSonar()
{
	if (GetController())
	{
		if (!bIsFishing)
		{
			UE_LOG(LogTemp, Warning, TEXT("Pling"));
			FishManager -> PlaySoundOnClosestFish(SonarSoundCue);
		}
	}
}

void AProd_grupp8Character::DoSwayRight()
{
	if (GetController())
	{
		if (bIsFishOnHook)
		{
			UE_LOG(LogTemp, Warning, TEXT("Right"));
		}
	}
}

void AProd_grupp8Character::DoSwayLeft()
{
	if (GetController())
	{
		if (bIsFishOnHook)
		{
			UE_LOG(LogTemp, Warning, TEXT("Left"));
		}
	}
}

void AProd_grupp8Character::ReelLineInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D ReelLineVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoReelLine(ReelLineVector.X, ReelLineVector.Y);

}

void AProd_grupp8Character::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		if (!bIsFishing)
		{
			// pass the rotation inputs
			AddControllerYawInput(Yaw * 0.2f);
			AddControllerPitchInput(Pitch * 0.2f);
		}
	}
}

void AProd_grupp8Character::DoReelLine(float Right, float Down)
{
	if (GetController())
	{
		if (bIsFishOnHook)
		{
			FVector2d NewReelStickPosition = FVector2d(Right, Down).GetSafeNormal();
			// Räkna ut vinkeln via arccosine av dot product
			float Dot = FVector2D::DotProduct(NewReelStickPosition, ReelStickPosition.GetSafeNormal());
			float AngleInRadians = FMath::Acos(Dot);
			
			ReelStickDegree = FMath::RadiansToDegrees(AngleInRadians);
			ReelStickPosition = NewReelStickPosition;
		}
	}
}

void AProd_grupp8Character::DoCastLine()
{
	if (GetController())
	{
		if (!bIsFishing && FishInRange)
		{
			UE_LOG(LogTemp, Warning, TEXT("Fish"));
			
			StartFishing();
		}
	}
}

void AProd_grupp8Character::StartFishing()
{
	bIsFishing = true;
	float fTimeToFish = FMath::RandRange(1.0, 3.0);
	FTimerHandle UnusedHandle;
	Bobber = SpawnBobber(this->GetActorLocation() + GetActorForwardVector() * 250);
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &AProd_grupp8Character::OnHook, fTimeToFish, false);
	UGameplayStatics::PlaySound2D(this, ThrowSoundCue, 1.0f, 1.0f, 0.0f);
}

//När man har fått napp (innan fiske)
void AProd_grupp8Character::OnHook()
{
	bIsFishOnHook = true;
	caughtProgress = 0.0f;
	UGameplayStatics::PlaySound2D(this, OnHookSoundCue, 1.0f, 1.0f, 0.0f);
	UE_LOG(LogTemp, Warning, TEXT("On Hook"));
	//FTimerHandle UnusedHandle;
	//GetWorldTimerManager().SetTimer(UnusedHandle, this, &AProd_grupp8Character::StartFishFighting, 2, false);
}

//(Under tiden man fiskar)
void AProd_grupp8Character::Fishing(float deltaTime)
{
	if (ReelStickDegree > 1.0f && ReelStickDegree < 3.0f)
	{
		ReelSoundTimer = 0.0f;
		if (ReelInSoundCueObject && !bReelSoundActive)
		{
			bReelSoundActive = true;
			ReelInSoundCueObject->FadeIn(0.2f, 1.0f);
		}
		
		UGameplayStatics::GetPlayerController(GetWorld(), 0)->ClientPlayForceFeedback(FishHapticEffect);
		caughtProgress += deltaTime * ReelStickDegree * 30.0f;
		if (caughtProgress >= 100)
		{
			Caught();
		}
		UE_LOG(LogTemp, Warning, TEXT("%f"), caughtProgress);
	}
	else if (ReelInSoundCueObject && bReelSoundActive)
	{
		if (ReelSoundTimer >= 0.6)
		{
			bReelSoundActive = false;
			ReelInSoundCueObject->FadeOut(0.2f, 0.0f);
		}
		else
		{
			ReelSoundTimer += deltaTime;
		}
	}
	
}

//När man fångat fisken (efter fiske)
void AProd_grupp8Character::Caught()
{
	
	bIsFishing = false;
	bIsFishOnHook = false;
	
	if (ReelInSoundCueObject && bReelSoundActive)
	{
		bReelSoundActive = false;
		ReelInSoundCueObject->FadeOut(0.2f, 0.0f);
	}
	
	UGameplayStatics::PlaySound2D(this, CaughtSoundCue, 1.0f, 1.0f, 0.0f);
	UE_LOG(LogTemp, Warning, TEXT("You caught the fish"));
	if (Bobber)
	{
		Bobber -> Destroy();
	}
	FishManager -> OnFishCaught(FishInRange);
	//UE_LOG(LogTemp, Warning, TEXT("You caught the fish %p"), Bobber);
	//Bobber -> Destroy();
}

AActor* AProd_grupp8Character::SpawnBobber(FVector SpawnLocation){
	FRotator SpawnRotation = FRotator::ZeroRotator;
	UWorld* World = GetWorld();
	if (!World || !BobberToSpawn)
	{
		return nullptr;
	}
	return World->SpawnActor<AActor>(BobberToSpawn, SpawnLocation, SpawnRotation);
}

void AProd_grupp8Character::StartFishFighting()
{
	bFishIsFighting = true;
	FishFightTimer = 0.0f;
}

void AProd_grupp8Character::FishFighting(float deltaTime)
{
	
}

void AProd_grupp8Character::FishFightingLeft(float deltaTime)
{
	Bobber->SetActorLocation(Bobber->GetActorLocation() + FVector(100 * deltaTime,0,0));
}

void AProd_grupp8Character::FishFightingRight(float deltaTime)
{
	Bobber->SetActorLocation(Bobber->GetActorLocation() + FVector(-100 * deltaTime,0,0));
}