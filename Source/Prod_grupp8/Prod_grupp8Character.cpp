// Copyright Epic Games, Inc. All Rights Reserved.

#include "Prod_grupp8Character.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Prod_grupp8.h"

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
		
		// Casting fishing line
		EnhancedInputComponent->BindAction(ReelLineAction, ETriggerEvent::Triggered, this, &AProd_grupp8Character::ReelLineInput);
		
		// Casting fishing line
		EnhancedInputComponent->BindAction(SonarAction, ETriggerEvent::Started, this, &AProd_grupp8Character::DoSonar);
	}
	else
	{
		UE_LOG(LogProd_grupp8, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
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
		Fishing(DeltaTime);
	}
}

void AProd_grupp8Character::DoSonar()
{
	if (GetController())
	{
		if (!bIsFishing)
		{
			UE_LOG(LogTemp, Warning, TEXT("Pling"));
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
		if (!bIsFishing)
		{
			UE_LOG(LogTemp, Warning, TEXT("Fish"));
		
			//checka om vi kan fiska
			StartFishing();
		}
	}
}

void AProd_grupp8Character::StartFishing()
{
	bIsFishing = true;
	float fTimeToFish = FMath::RandRange(1.0, 3.0);
	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &AProd_grupp8Character::OnHook, fTimeToFish, false);

}

//När man har fått napp (innan fiske)
void AProd_grupp8Character::OnHook()
{
	bIsFishOnHook = true;
	UE_LOG(LogTemp, Warning, TEXT("On Hook"));
}

//(Under tiden man fiskar)
void AProd_grupp8Character::Fishing(float deltaTime)
{
	if (ReelStickDegree > 1.0f && ReelStickDegree < 3.0f)
	{
		caughtProgress += deltaTime * ReelStickDegree * 30.0f;
		if (caughtProgress >= 100)
		{
			Caught();
		}
		UE_LOG(LogTemp, Warning, TEXT("%f"), caughtProgress);
	}
}

//När man fångat fisken (efter fiske)
void AProd_grupp8Character::Caught()
{
	bIsFishing = false;
	bIsFishOnHook = false;
	UE_LOG(LogTemp, Warning, TEXT("You caught the fish"));
}

