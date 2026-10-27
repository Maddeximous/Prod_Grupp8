// Copyright Epic Games, Inc. All Rights Reserved.

#include "Prod_grupp8Character.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "FishManager.h"
#include "Inventory.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Prod_grupp8.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "GameTelemetry.h"
#include "Shop.h"
#include "OnboardingSubsystem.h"

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
		
		//Pause
		EnhancedInputComponent->BindAction(OpenPauseMenuAction, ETriggerEvent::Started, this, &AProd_grupp8Character::DoOpenPauseMenu);
		
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AProd_grupp8Character::MoveInput);

		// Ändra från Triggered till Started här:
		EnhancedInputComponent->BindAction(ShopTransactionAction, ETriggerEvent::Started, this, &AProd_grupp8Character::DoShopTransaction);
		
		// Ändra mellan fisk och fiskespö i shop menyn
		EnhancedInputComponent->BindAction(SwitchShopViewAction, ETriggerEvent::Started, this, &AProd_grupp8Character::DoSwitchShopView);
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
	PauseMenu = Cast<APauseMenu>(UGameplayStatics::GetActorOfClass(this, APauseMenu::StaticClass()));

	if (MoveForwardSoundCue)
	{
		// Loops for the whole session; only the pitch changes. Not auto-destroyed.
		CurrentMovePitch = IdleMovePitch;
		PreviousYaw = GetActorRotation().Yaw;
		MoveForwardSoundObject = UGameplayStatics::SpawnSound2D(this, MoveForwardSoundCue, 1.0f, IdleMovePitch, 0.0f, nullptr, true, false);
	}
	
}

void AProd_grupp8Character::UpdateMoveSound(float DeltaTime)
{
	if (!MoveForwardSoundObject)
	{
		return;
	}

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	const FVector Acceleration = Movement->GetCurrentAcceleration();

	// Forward and backward movement both count: acceleration along the actor's forward axis (sign ignored)
	const float ForwardAccel = FMath::Abs(FVector::DotProduct(Acceleration, GetActorForwardVector()));
	const bool bMoving = !bIsFishing && Movement->IsMovingOnGround() && ForwardAccel > KINDA_SMALL_NUMBER;

	// Turn speed in degrees/sec, measured from the actor's yaw change since last tick
	const float CurrentYaw = GetActorRotation().Yaw;
	// Signed: positive = turning right, negative = turning left
	const float TurnRate = DeltaTime > KINDA_SMALL_NUMBER
		? FMath::FindDeltaAngleDegrees(PreviousYaw, CurrentYaw) / DeltaTime
		: 0.0f;
	PreviousYaw = CurrentYaw;

	// Stationary -> idle pitch (slightly raised while turning on the spot);
	// moving forward or backward -> pitch follows acceleration
	float TargetPitch = IdleMovePitch;
	if (bMoving)
	{
		// 0..1 acceleration relative to the maximum the movement component allows
		const float AccelRatio = FMath::Clamp(ForwardAccel / FMath::Max(Movement->GetMaxAcceleration(), 1.0f), 0.0f, 1.0f);
		TargetPitch = FMath::Lerp(MinMovePitch, MaxMovePitch, AccelRatio);
	}
	else if (!bIsFishing)
	{
		const float TurnRatio = FMath::Clamp(FMath::Abs(TurnRate) / FMath::Max(MaxTurnRateForPitch, 1.0f), 0.0f, 1.0f);
		const float TurnBoost = TurnRate > 0.0f ? TurnRightPitchBoost : TurnLeftPitchBoost;
		TargetPitch = IdleMovePitch + TurnBoost * TurnRatio;
	}

	CurrentMovePitch = FMath::FInterpTo(CurrentMovePitch, TargetPitch, DeltaTime, MovePitchInterpSpeed);
	MoveForwardSoundObject->SetPitchMultiplier(CurrentMovePitch);
}

void AProd_grupp8Character::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	if (ShopInRange && ShopInRange -> shopEnabled)
	{
		if (MovementVector.Y > 0.5)
		{
			ShopInRange -> BuyNextRod();
		}
		else if (MovementVector.Y < -0.5)
		{
			ShopInRange -> SellAllFish();
		}
	}
	else
	{
		DoMove(0, MovementVector.Y);
	}
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
	UpdateMoveSound(DeltaTime);
	if (bIsFishOnHook)
	{
		float ReelValue = Reeling(DeltaTime);
		
		if (bFishIsFighting)
		{
			
			FishFighting(DeltaTime, ReelValue);
		}
		else
		{
			Fishing(DeltaTime, ReelValue);
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
			UOnboardingSubsystem::ReportOnboardingEvent(this, TEXT("SonarPinged"));
			FishManager -> PlaySoundOnClosestFish(SonarSoundCue);
			if (UGameTelemetry* Telemetry = GetGameInstance()->GetSubsystem<UGameTelemetry>())
			{
				Telemetry->RecordAButtonPress();
			}
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
		// Om shoppen är öppen, lås blicken/kameran
		if (ShopInRange && ShopInRange->shopEnabled)
		{
			return;
		}

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
			FVector2d NewReelStickPosition = FVector2d(Right, Down);
			if (NewReelStickPosition.Length() < 0.8) {NewReelStickPosition = FVector2D(0.0f, 0.0f);}
			// Räkna ut vinkeln via arccosine av dot product
			float Dot = FVector2D::DotProduct(NewReelStickPosition.GetSafeNormal(), ReelStickPosition.GetSafeNormal());
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
		// Kolla om shoppen är öppen via ShopInRange
		if (ShopInRange)
		{
			ShopInRange -> ToggleShop();
		}
		else if (!bIsFishing && FishInRange)
		{
			UE_LOG(LogTemp, Warning, TEXT("Fish"));
			
			StartFishing();
		}
	}
}

void AProd_grupp8Character::DoOpenPauseMenu()
{
	//PauseMenu -> TogglePauseMenu();
	UE_LOG(LogTemp, Warning, TEXT("Ingen meny. Shit är knas"));
}

void AProd_grupp8Character::StartFishing()
{
	UE_LOG(LogTemp, Warning, TEXT("Start Fish"));
	bIsFishing = true;
	float fTimeToFish = FMath::RandRange(2.0, 5.0);
	FTimerHandle UnusedHandle;
	Bobber = SpawnBobber(this->GetActorLocation() + GetActorForwardVector() * 250);
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &AProd_grupp8Character::OnHook, fTimeToFish, false);
	UGameplayStatics::PlaySound2D(this, ThrowSoundCue, 1.0f, 1.0f, 0.0f);
	FishInRange -> FishingAudioComp -> FadeOut(0.5f, 0.0f);
	bReelSoundActive = false;
	ReelSoundTimer = 0.0f;

	if (ReelInSoundCue && !IsValid(ReelInSoundCueObject))
	{
		ReelInSoundCueObject = UGameplayStatics::CreateSound2D(this, ReelInSoundCue, 1.0f, 1.0f, 0.0f, nullptr, false, false);
	}
	if (TensionSoundCue && !IsValid(TensionSoundCueObject))
	{
		TensionSoundCueObject = UGameplayStatics::CreateSound2D(this, TensionSoundCue, 1.0f, 1.0f, 0.0f, nullptr, false, false);
	}
}

//När man har fått napp (innan fiske)
void AProd_grupp8Character::OnHook()
{
	FTimerHandle UnusedHandle;
	float NextFishFight = FMath::RandRange(3.0, 5.0);
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &AProd_grupp8Character::StartFishFighting, NextFishFight, false);
	
	ProgressToCatch = 50 * FishManager -> FishDataBase[FishInRange -> FishSpecies].CatchDifficulty + FishInRange -> Weight * 5.0f;
	LineDurability = 100.0;
	caughtProgress = 0.0f;
	bIsFishOnHook = true;
	bFishIsFighting = false;
	
	UGameplayStatics::PlaySound2D(this, OnHookSoundCue, 1.0f, 1.0f, 0.0f);
	UE_LOG(LogTemp, Warning, TEXT("On Hook: %f"), ProgressToCatch);
	//FTimerHandle UnusedHandle;
	//GetWorldTimerManager().SetTimer(UnusedHandle, this, &AProd_grupp8Character::StartFishFighting, 2, false);
	if (UGameTelemetry* Telemetry = GetGameInstance()->GetSubsystem<UGameTelemetry>())
	{
		Telemetry->StartReeling();
	}
}

//När man fångat fisken (efter fiske)
void AProd_grupp8Character::Caught()
{
	
	bIsFishing = false;
	bIsFishOnHook = false;
	
	const bool bCaughtGhostTreasure = IsValid(FishInRange) && FishInRange->FishSpecies == EFishType::Spokskatt;
	
	if (ReelInSoundCueObject && bReelSoundActive)
	{
		bReelSoundActive = false;
		ReelInSoundCueObject->FadeOut(0.2f, 0.0f);
	}
	
	UGameplayStatics::PlaySound2D(this, CaughtSoundCue, 1.0f, 1.0f, 0.0f);
	UE_LOG(LogTemp, Warning, TEXT("You caught the fish"));
	UOnboardingSubsystem::ReportOnboardingEvent(this, TEXT("FishCaught"));
	if (Bobber)
	{
		Bobber -> Destroy();
	}
	
	AFish* CaughtFish = FishInRange;
	const bool bHasCaughtFish = IsValid(CaughtFish);

	EFishType CaughtSpecies = EFishType::None;
	float CaughtWeight = 0.0f;
	float CaughtTotalPrice = 0.0f;

	if (bHasCaughtFish)
	{
		CaughtSpecies = CaughtFish->FishSpecies;
		CaughtWeight = CaughtFish->Weight;
		
		FString SpeciesNameStr = UEnum::GetValueAsString(CaughtSpecies);
		UE_LOG(LogTemp, Warning, TEXT("Du fångade fiskart: %s"), *SpeciesNameStr);
		
		if (IsValid(FishManager))
		{
			if (const FFishData* FishInfo = FishManager->FishDataBase.Find(CaughtSpecies))
			{
				CaughtTotalPrice = CaughtWeight * FishInfo->PricePerKilo;

				if (FishInfo->SpeciesNameAudio)
				{
					UGameplayStatics::PlaySound2D(this, FishInfo->SpeciesNameAudio, 1.0f, 1.0f, 0.0f);
					UE_LOG(LogTemp, Warning, TEXT("Spelar upp namnljud för fisk!"));
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("FishInfo hittades, men SpeciesNameAudio var nullptr (None)!"));
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Hittade inte fisken i FishDataBase!"));
			}
		}
	}
	if (IsValid(FishManager) && bHasCaughtFish)
	{
		FishManager->OnFishCaught(CaughtFish);
	}
	
	FishCaught++;
	//UE_LOG(LogTemp, Warning, TEXT("You caught the fish %p"), Bobber);
	//Bobber -> Destroy();
	if (UGameTelemetry* Telemetry = GetGameInstance()->GetSubsystem<UGameTelemetry>())
	{
		Telemetry->EndReeling(true);
	}
	
	//Blueprintcallable event for the onboarding
	if (bHasCaughtFish)
	{
		OnFishCaughtWithInfo(
			CaughtSpecies,
			CaughtWeight,
			CaughtTotalPrice);
	}
	if (bCaughtGhostTreasure)
	{
		OnGhostTreasureCaught();
	}
}

void AProd_grupp8Character::Escape()
{
	
	bIsFishing = false;
	bIsFishOnHook = false;
	
	if (ReelInSoundCueObject && bReelSoundActive)
	{
		bReelSoundActive = false;
		ReelInSoundCueObject->FadeOut(0.2f, 0.0f);
	}
	
	UGameplayStatics::PlaySound2D(this, EscapeSoundCue, 2.0f, 1.0f, 0.0f);
	UE_LOG(LogTemp, Warning, TEXT("The fish got away"));
	
	if (Bobber)
	{
		Bobber -> Destroy();
	}
	if (UGameTelemetry* Telemetry = GetGameInstance()->GetSubsystem<UGameTelemetry>())
	{
		Telemetry->EndReeling(true);
	}
	
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
	if (bIsFishOnHook)
	{
		bFishIsFighting = true;
		FishFightTimer = FishManager -> FishDataBase[FishInRange -> FishSpecies].CatchDifficulty * 4.0f;
		UGameplayStatics::PlaySound2D(this, FightSoundCue, 1.0f, 1.0f, 0.0f);
		if (TensionSoundCueObject) {TensionSoundCueObject -> Play();}
		UE_LOG(LogTemp, Warning, TEXT("The fish is fighting back!"));
	}
}

//(Under tiden man fiskar)
void AProd_grupp8Character::Fishing(float DeltaTime, float ReelValue)
{
	caughtProgress += ReelValue;
	UE_LOG(LogTemp, Warning, TEXT("Fishing: %f"), caughtProgress);
	if (caughtProgress >= ProgressToCatch)
	{
		Caught();
	}
}

float AProd_grupp8Character::Reeling(float DeltaTime)
{
	if (ReelStickDegree > 1.0f && ReelStickDegree < 3.0f)
	{
		ReelSoundTimer = 0.0f;
		if (ReelInSoundCueObject && !bReelSoundActive)
		{
			bReelSoundActive = true;
			ReelInSoundCueObject->FadeIn(0.4f, 0.6f);
		}
		
		return DeltaTime * ReelStickDegree * 10.0f;
	}
	if (ReelInSoundCueObject && bReelSoundActive)
	{
		if (ReelSoundTimer >= 0.6)
		{
			bReelSoundActive = false;
			ReelInSoundCueObject->FadeOut(0.35f, 0.0f);
		}
		else
		{
			ReelSoundTimer += DeltaTime;
		}
	}
	return 0.0;
}

void AProd_grupp8Character::FishFighting(float DeltaTime, float ReelValue)
{
	FishFightTimer -= DeltaTime;
	if (FishFightTimer < 0.0)
	{
		bFishIsFighting = false;
		FTimerHandle UnusedHandle;
		float NextFishFight = FMath::RandRange(5.0, 9.0);
		GetWorldTimerManager().SetTimer(UnusedHandle, this, &AProd_grupp8Character::StartFishFighting, NextFishFight, false);
		if (TensionSoundCueObject) {TensionSoundCueObject -> Stop();}
		LineDurability += 20.0f;
	}
	
	LineDurability -= FishManager -> FishDataBase[FishInRange -> FishSpecies].CatchDifficulty * ReelValue * 13.0f;
	if (TensionSoundCueObject) {TensionSoundCueObject -> SetPitchMultiplier(4.0f - LineDurability * (3.0f / 100.0f));}
	//UE_LOG(LogTemp, Warning, TEXT("Pitch %f"), TensionSoundCueObject -> PitchMultiplier);
	if (LineDurability <= 0.0)
	{
		UE_LOG(LogTemp, Warning, TEXT("The fish is loose"));
		Escape();
		if (TensionSoundCueObject) {TensionSoundCueObject -> Stop();}
	}
	
	UGameplayStatics::GetPlayerController(GetWorld(), 0)->ClientPlayForceFeedback(ReelHapticEffect);
}

void AProd_grupp8Character::DoShopTransaction()
{
	if (ShopInRange)
	{
		ShopInRange->Transaction();
	}
}

void AProd_grupp8Character::DoSwitchShopView()
{
	if (ShopInRange && ShopInRange->shopEnabled)
	{
		ShopInRange->ToggleSection();
	}
}