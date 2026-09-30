// Copyright Epic Games, Inc. All Rights Reserved.


#include "Prod_grupp8PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Prod_grupp8CameraManager.h"
#include "Blueprint/UserWidget.h"
#include "Prod_grupp8.h"
#include "Widgets/Input/SVirtualJoystick.h"

AProd_grupp8PlayerController::AProd_grupp8PlayerController()
{
	// set the player camera manager class
	PlayerCameraManagerClass = AProd_grupp8CameraManager::StaticClass();
}

void AProd_grupp8PlayerController::BeginPlay()
{
	Super::BeginPlay();

	
	// only spawn touch controls on local player controllers
	if (IsLocalPlayerController() && ShouldUseTouchControls())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogProd_grupp8, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void AProd_grupp8PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Context
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
	
}

bool AProd_grupp8PlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}
