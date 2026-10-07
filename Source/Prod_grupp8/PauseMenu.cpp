// Fill out your copyright notice in the Description page of Project Settings.


#include "PauseMenu.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"
#include "InputCoreTypes.h"
#include "SpeechSubsystem.h"

// Sets default values
APauseMenu::APauseMenu()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
}

// Called when the game starts or when spawned
void APauseMenu::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetWorld()->GetFirstPlayerController();

	if (!PC)
		return;

	EnableInput(PC);
	

	if (!InputComponent)
		return;

	InputComponent->BindKey(
		EKeys::Up,
		IE_Pressed,
		this,
		&APauseMenu::MoveUp
	);

	InputComponent->BindKey(
		EKeys::Down,
		IE_Pressed,
		this,
		&APauseMenu::MoveDown
	);
	InputComponent->BindKey(
		EKeys::Enter,
		IE_Pressed,
		this,
		&APauseMenu::Select
	);

	InputComponent->BindKey(
		EKeys::Q,
		IE_Pressed,
		this,
		&APauseMenu::GoBack
	);
}

void APauseMenu::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bPaused != bPreviousPaused)
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();

		if (PC)
		{
			if (bPaused)
			{
				EnableInput(PC);
			}
			else
			{
				DisableInput(PC);
			}
		}

		// Reset menu whenever the state changes
		CurrentMenuIndex = 0;
		SelectedItemIndex = 0;
		MenuHistory.Empty();

		// Remove debug text when disabled
		if (!bPaused && GEngine)
		{
			GEngine->RemoveOnScreenDebugMessage(4);
		}

		bPreviousPaused = bPaused;
	}

	if (bPaused)
	{
		UpdateDebugText();
	}
}

void APauseMenu::MoveUp()
{
	if (Menus.IsEmpty())
		return;

	const FMenuData& CurrentMenu = Menus[CurrentMenuIndex];

	if (CurrentMenu.Items.IsEmpty())
		return;

	SelectedItemIndex--;

	if (SelectedItemIndex < 0)
	{
		SelectedItemIndex = CurrentMenu.Items.Num() - 1;
	}
	
	VoiceState();
}

void APauseMenu::MoveDown()
{
	if (Menus.IsEmpty())
		return;

	const FMenuData& CurrentMenu = Menus[CurrentMenuIndex];

	if (CurrentMenu.Items.IsEmpty())
		return;

	SelectedItemIndex++;

	if (SelectedItemIndex >= CurrentMenu.Items.Num())
	{
		SelectedItemIndex = 0;
	}
	VoiceState();
	
}

void APauseMenu::Select()
{
	if (!Menus.IsValidIndex(CurrentMenuIndex))
		return;

	FMenuData& CurrentMenu = Menus[CurrentMenuIndex];

	if (!CurrentMenu.Items.IsValidIndex(SelectedItemIndex))
		return;

	FPauseMenuItem& Item = CurrentMenu.Items[SelectedItemIndex];

	if (Item.Type == EPauseMenuItemType::Submenu)
	{
		if (Menus.IsValidIndex(Item.SubmenuIndex))
		{
			// Remember where we came from
			MenuHistory.Add(CurrentMenuIndex);

			// Enter submenu
			CurrentMenuIndex = Item.SubmenuIndex;
			SelectedItemIndex = 0;
		}
	VoiceState();
		
	}
	else
	{
		ExecuteAction(Item.Action);
	}
}

void APauseMenu::ExecuteAction(EPauseMenuAction Action)
{
	switch (Action)
	{
	case EPauseMenuAction::Resume:
		bPaused= false;
		// Resume game
		break;

	case EPauseMenuAction::Restart:
		// Restart level
		break;

	case EPauseMenuAction::Quit:
		// Quit game
		break;

	case EPauseMenuAction::OpenSettings:
		// Open settings
		break;

	case EPauseMenuAction::None:
		break;
	}
}

void APauseMenu::GoBack()
{
	if (MenuHistory.IsEmpty())
		return;

	CurrentMenuIndex = MenuHistory.Last();
	MenuHistory.Pop();

	SelectedItemIndex = 0;
	VoiceState();
	
}

void APauseMenu::UpdateDebugText()
{
	if (!GEngine)
		return;

	if (!Menus.IsValidIndex(CurrentMenuIndex))
		return;

	const FMenuData& CurrentMenu = Menus[CurrentMenuIndex];

	FString MenuName = CurrentMenu.Name.ToString();
	FString ItemName = TEXT("None");

	if (CurrentMenu.Items.IsValidIndex(SelectedItemIndex))
	{
		ItemName = CurrentMenu.Items[SelectedItemIndex].Name.ToString();
	}

	FString DebugText = FString::Printf(
		TEXT("                                                                                                   Menu: %s | Selected: %s"),
		*MenuName,
		*ItemName
	);

	GEngine->AddOnScreenDebugMessage(
		4,
		2.0f,
		FColor::Purple,
		DebugText
	);
	
}

bool APauseMenu::IsPaused() const
{
	return bPaused;
}

void APauseMenu::TogglePauseMenu()
{
	bPaused = !bPaused;

	// Reset menu state
	CurrentMenuIndex = 0;
	SelectedItemIndex = 0;
	MenuHistory.Empty();

	if (bPaused)
	{
		EnableInput(GetWorld()->GetFirstPlayerController());
	}
	else
	{
		DisableInput(GetWorld()->GetFirstPlayerController());

		// Remove debug text immediately
		if (GEngine)
		{
			GEngine->RemoveOnScreenDebugMessage(4);
		}
	}
}

void APauseMenu::VoiceState()
{
	if (!GEngine)
		return;

	if (!Menus.IsValidIndex(CurrentMenuIndex))
		return;

	const FMenuData& CurrentMenu = Menus[CurrentMenuIndex];

	FString MenuName = CurrentMenu.Name.ToString();
	FString ItemName = TEXT("None");

	if (CurrentMenu.Items.IsValidIndex(SelectedItemIndex))
	{
		ItemName = CurrentMenu.Items[SelectedItemIndex].Name.ToString();
	}
	
	
	UGameInstance* GI = GetGameInstance();
	USpeechSubsystem* Speech = GI ? GI->GetSubsystem<USpeechSubsystem>() : nullptr;
	if (!Speech)
	{
		return;
	}
	Speech->Speak(FText::FromString(ItemName),true);
}