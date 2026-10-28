// Fill out your copyright notice in the Description page of Project Settings.


#include "PauseMenu.h"
#include "Inventory.h"
#include "Fish.h"
#include "Blueprint/UserWidget.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "NumberSpeaker.h"
#include "FishManager.h"
#include "Prod_grupp8Character.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"
#include "InputCoreTypes.h"

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

	// Navigate up
	InputComponent->BindKey(
		EKeys::Gamepad_DPad_Up,
		IE_Pressed,
		this,
		&APauseMenu::MoveUp
	);

	// Navigate down
	InputComponent->BindKey(
		EKeys::Gamepad_DPad_Down,
		IE_Pressed,
		this,
		&APauseMenu::MoveDown
	);

	// Select (A / Cross)
	InputComponent->BindKey(
		EKeys::Gamepad_FaceButton_Bottom,
		IE_Pressed,
		this,
		&APauseMenu::Select
	);

	// Go back (B / Circle)
	InputComponent->BindKey(
		EKeys::Gamepad_FaceButton_Right,
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

void APauseMenu::OpenSettings()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();

	if (!PC || !SettingsWidgetClass)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Settings widget class or PlayerController is missing"));
		return;
	}

	// Don't create the widget multiple times
	if (SettingsWidget && SettingsWidget->IsInViewport())
	{
		return;
	}

	SettingsWidget = CreateWidget<UUserWidget>(
		PC,
		SettingsWidgetClass
	);

	if (!SettingsWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create settings widget"));
		return;
	}

	SettingsWidget->AddToViewport();

	// Allow the player to interact with the widget
	PC->SetInputMode(FInputModeGameAndUI());
	PC->bShowMouseCursor = true;

	// Give the widget keyboard/gamepad focus
	SettingsWidget->SetIsFocusable(true);
	PC->SetInputMode(FInputModeGameAndUI().SetWidgetToFocus(
		SettingsWidget->TakeWidget()
	));
}

void APauseMenu::PlayNavigationSound()
{
	if (!bPaused) return;
	if (Menus.IsEmpty() || !Menus.IsValidIndex(CurrentMenuIndex))
	{
		return;
	}

	const FMenuData& CurrentMenu = Menus[CurrentMenuIndex];

	if (!CurrentMenu.Items.IsValidIndex(SelectedItemIndex))
	{
		return;
	}

	const FPauseMenuItem& Item = CurrentMenu.Items[SelectedItemIndex];
	const EPauseMenuAction Action = Item.Action;

	USoundBase* SoundToPlay = nullptr;

	switch (Action)
	{
	case EPauseMenuAction::Resume:
		SoundToPlay = ResumeSound;
		break;

	case EPauseMenuAction::OpenSettings:
		SoundToPlay = OptionsSound;
		break;

	case EPauseMenuAction::OpenFishInventory:
		SoundToPlay = InventorySound;
		break;

	case EPauseMenuAction::Quit:
		SoundToPlay = QuitSound;
		break;

	default:
		// Handle menu items whose Action is None.
		if (Item.Name.ToString().Equals(
			TEXT("Options"), ESearchCase::IgnoreCase))
		{
			SoundToPlay = OptionsSound;
		}
		else if (Item.Name.ToString().Equals(
			TEXT("Resume Game"), ESearchCase::IgnoreCase))
		{
			SoundToPlay = ResumeSound;
		}
		else if (Item.Name.ToString().Equals(
			TEXT("Exit Game"), ESearchCase::IgnoreCase))
		{
			SoundToPlay = QuitSound;
		}
		break;
	}

	UE_LOG(LogTemp, Warning,
		TEXT("Narration: Item = %s | Action = %d | Sound assigned = %s"),
		*Item.Name.ToString(),
		static_cast<int32>(Action),
		SoundToPlay ? TEXT("YES") : TEXT("NO"));

	// Stop the previous narration before playing the new one.
	if (CurrentNarrationComponent)
	{
		CurrentNarrationComponent->Stop();
		CurrentNarrationComponent = nullptr;
	}

	if (SoundToPlay)
	{
		CurrentNarrationComponent =
			UGameplayStatics::SpawnSound2D(this, SoundToPlay);
	}
}

void APauseMenu::CloseSettings()
{
	if (SettingsWidget)
	{
		SettingsWidget->RemoveFromParent();
		SettingsWidget = nullptr;
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();

	if (PC)
	{
		// Return input to the pause menu
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
	}

	// Keep the game paused!
	bPaused = true;
}

void APauseMenu::MoveUp()
{
	if (!bPaused) return;
	
	if (bBrowsingFishInventory)
	{
		BrowseFishUp();
		ReadSelectedFish();
		return;
	}


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

	PlayNavigationSound();
}

void APauseMenu::BrowseFishUp()
{
	if (FishList.IsEmpty())
		return;

	CurrentFishIndex--;

	if (CurrentFishIndex < 0)
	{
		CurrentFishIndex = FishList.Num() - 1;
	}
}

void APauseMenu::BrowseFishDown()
{
	if (FishList.IsEmpty())
		return;

	CurrentFishIndex++;

	if (CurrentFishIndex >= FishList.Num())
	{
		CurrentFishIndex = 0;
	}
}

void APauseMenu::MoveDown()
{
	if (!bPaused) return;
	
	
	
	if (bBrowsingFishInventory)
	{
		BrowseFishDown();
		ReadSelectedFish();
		return;
	}


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

	PlayNavigationSound();
}

void APauseMenu::Select()
{
	if (!bPaused) return;
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

			// Check if this is the Fish Inventory submenu
			if (Menus[CurrentMenuIndex].Name.ToString()
				== TEXT("Fish Inventory"))
			{
				OpenFishInventory();
			}
			else
			{
				bBrowsingFishInventory = false;
			}
		}

		// Speak the current selection
		if (bBrowsingFishInventory)
		{
			// Fish selection is displayed by UpdateDebugText()
			// and can be spoken here if desired.
		}
		else
		{
		}
	}
	else
	{
		ExecuteAction(Item.Action);
	}
}

void APauseMenu::ExecuteAction(EPauseMenuAction Action)
{
	UE_LOG(LogTemp, Warning,
		TEXT("ExecuteAction called: %d"),
		static_cast<int32>(Action));

	switch (Action)
	{
	case EPauseMenuAction::OpenFishInventory:
		UE_LOG(LogTemp, Warning, TEXT("Opening fish inventory"));
		OpenFishInventory();
		break;

	case EPauseMenuAction::Resume:
        // Unpause the game and restore normal time and input.
        if (bPaused)
        {
            TogglePauseMenu();
        }
        break;

	case EPauseMenuAction::Restart:
		break;

	case EPauseMenuAction::Quit:
        // Restore normal time before loading the main menu.
        bPaused = false;
    
        if (UWorld* World = GetWorld())
        {
            UGameplayStatics::SetGlobalTimeDilation(World, 1.0f);
            UGameplayStatics::OpenLevel(
                this,
                FName(TEXT("Level_Main_Menu"))
            );
        }
        break;

	case EPauseMenuAction::OpenSettings:
		OpenSettings();
		break;

	case EPauseMenuAction::None:
		break;
		
	}
}

void APauseMenu::FindNumberSpeaker()
{
	if (IsValid(NumberSpeaker))
	{
		return;
	}

	for (TActorIterator<ANumberSpeaker> It(GetWorld()); It; ++It)
	{
		NumberSpeaker = *It;
		break;
	}
}

void APauseMenu::StopFishNarration()
{
	GetWorldTimerManager().ClearTimer(FishNarrationTimer);

	if (IsValid(FishNarrationComponent))
	{
		FishNarrationComponent->Stop();
		FishNarrationComponent = nullptr;
	}

	// Also stop NumberSpeaker's active audio here if it has
	// its own audio component and stop function.
}




void APauseMenu::ReadSelectedFish()
{
	if (!bPaused || !bBrowsingFishInventory)
	{
		return;
	}

	if (!FishList.IsValidIndex(CurrentFishIndex))
	{
		return;
	}

	AFish* Fish = FishList[CurrentFishIndex].Get();

	if (!IsValid(Fish))
	{
		return;
	}

	UWorld* World = GetWorld();
	APlayerController* PC = World
		? World->GetFirstPlayerController()
		: nullptr;

	AProd_grupp8Character* Character = PC
		? Cast<AProd_grupp8Character>(PC->GetCharacter())
		: nullptr;

	if (IsValid(Character))
	{
		Character->ReadFish(Fish);
	}
}




void APauseMenu::ReadSelectedFishWeight()
{
	if (!bPaused || !bBrowsingFishInventory)
		return;

	const float Duration =
		NumberSpeaker->SpeakKilo(SelectedFishWeight, 0, true);

	GetWorldTimerManager().SetTimer(
		FishNarrationTimer,
		this,
		&APauseMenu::ReadSelectedFishPrice,
		Duration,
		false
	);
}

void APauseMenu::ReadSelectedFishPrice()
{
	if (!bPaused || !bBrowsingFishInventory)
		return;

	NumberSpeaker->SpeakSek(SelectedFishPrice, 0, true);
}


void APauseMenu::OpenFishInventory()
{
    UE_LOG(LogTemp, Warning, TEXT("=== OPEN FISH INVENTORY ==="));

    if (!IsValid(Inventory))
    {
        UE_LOG(LogTemp, Error, TEXT("Inventory is INVALID"));
        return;
    }

    if (!Menus.IsValidIndex(2))
    {
        UE_LOG(LogTemp, Error, TEXT("Fish menu index 2 does not exist"));
        return;
    }

    FMenuData& FishMenu = Menus[2];

    // Clear both lists before rebuilding the inventory.
    FishMenu.Items.Empty();
    FishList.Empty();

    // Add each valid fish to BOTH the menu and the fish list.
    for (AFish* Fish : Inventory->Fish)
    {
        if (!IsValid(Fish))
        {
            continue;
        }

        // This is the missing line that lets ReadSelectedFish find the fish.
        FishList.Add(Fish);

        FString FishTypeName;

        switch (Fish->FishSpecies)
        {
        case EFishType::Aborre:     FishTypeName = TEXT("Aborre"); break;
        case EFishType::Gadda:      FishTypeName = TEXT("Gädda"); break;
        case EFishType::Gos:        FishTypeName = TEXT("Gös"); break;
        case EFishType::Insjooring: FishTypeName = TEXT("Insjööring"); break;
        case EFishType::Roding:     FishTypeName = TEXT("Röding"); break;
        case EFishType::Regnbage:   FishTypeName = TEXT("Regnbåge"); break;
        case EFishType::Sik:        FishTypeName = TEXT("Sik"); break;
        case EFishType::Lake:       FishTypeName = TEXT("Lake"); break;
        case EFishType::Braxen:     FishTypeName = TEXT("Braxen"); break;
        case EFishType::Sutare:     FishTypeName = TEXT("Sutare"); break;
        default:                    FishTypeName = TEXT("Unknown"); break;
        }

        FPauseMenuItem NewItem;

        NewItem.Name = FText::FromString(
            FString::Printf(
                TEXT("%s | Weight: %.2f kg"),
                *FishTypeName,
                Fish->Weight
            )
        );

        NewItem.Type = EPauseMenuItemType::Action;
        NewItem.Action = EPauseMenuAction::None;

        FishMenu.Items.Add(NewItem);
    }

    CurrentMenuIndex = 2;
    SelectedItemIndex = 0;
    CurrentFishIndex = 0;

    // Only enable browsing after the lists have been populated.
    bBrowsingFishInventory = !FishList.IsEmpty();

    UE_LOG(LogTemp, Warning,
        TEXT("Fish menu items: %d | FishList items: %d | Browsing: %s"),
        FishMenu.Items.Num(),
        FishList.Num(),
        bBrowsingFishInventory ? TEXT("true") : TEXT("false"));

    if (bBrowsingFishInventory)
    {
        ReadSelectedFish();
    }
}




void APauseMenu::GoBack()
{
	if (!bPaused)
	{
		return;
	}

	// If we're in the fish inventory, leave it first
	if (bBrowsingFishInventory)
	{
		bBrowsingFishInventory = false;
		FishList.Empty();

		// Reset the selected item
		SelectedItemIndex = 0;

		// Return to the main pause menu
		CurrentMenuIndex = 0;

		// Clear the menu history so we don't pop an invalid entry
		MenuHistory.Empty();

		UE_LOG(LogTemp, Warning, TEXT("Returned to pause menu."));
		return;
	}

	// Otherwise, return to the previous menu if one exists
	if (!MenuHistory.IsEmpty())
	{
		CurrentMenuIndex = MenuHistory.Pop();
		SelectedItemIndex = 0;
	}
}



void APauseMenu::UpdateDebugText()
{
    if (!GEngine)
        return;

    FString DebugText;

    // Display fish inventory when browsing fish
    if (bBrowsingFishInventory)
    {
        if (FishList.IsEmpty())
        {
            DebugText = TEXT("Fish Inventory | No fish in inventory");
        }
        else if (FishList.IsValidIndex(CurrentFishIndex))
        {
            AFish* Fish = FishList[CurrentFishIndex].Get();

            if (IsValid(Fish))
            {
                FString FishTypeName;

                switch (Fish->FishSpecies)
                {
                case EFishType::Aborre:
                    FishTypeName = TEXT("Aborre");
                    break;

                case EFishType::Gadda:
                    FishTypeName = TEXT("Gädda");
                    break;

                case EFishType::Gos:
                    FishTypeName = TEXT("Gös");
                    break;

                case EFishType::Insjooring:
                    FishTypeName = TEXT("Insjööring");
                    break;

                case EFishType::Roding:
                    FishTypeName = TEXT("Röding");
                    break;

                case EFishType::Regnbage:
                    FishTypeName = TEXT("Regnbåge");
                    break;

                case EFishType::Sik:
                    FishTypeName = TEXT("Sik");
                    break;

                case EFishType::Lake:
                    FishTypeName = TEXT("Lake");
                    break;

                case EFishType::Braxen:
                    FishTypeName = TEXT("Braxen");
                    break;

                case EFishType::Sutare:
                    FishTypeName = TEXT("Sutare");
                    break;

                default:
                    FishTypeName = TEXT("Unknown");
                    break;
                }

                DebugText = FString::Printf(
                    TEXT("Fish Inventory | Fish %d of %d | %s | Weight: %.2f kg"),
                    CurrentFishIndex + 1,
                    FishList.Num(),
                    *FishTypeName,
                    Fish->Weight
                );
            }
            else
            {
                DebugText = TEXT("Fish Inventory | Invalid fish");
            }
        }
    }
    else
    {
        // Normal pause menu display
        if (!Menus.IsValidIndex(CurrentMenuIndex))
            return;

        const FMenuData& CurrentMenu = Menus[CurrentMenuIndex];

        FString MenuName = CurrentMenu.Name.ToString();
        FString ItemName = TEXT("None");

        if (CurrentMenu.Items.IsValidIndex(SelectedItemIndex))
        {
            ItemName =
                CurrentMenu.Items[SelectedItemIndex].Name.ToString();
        }

        DebugText = FString::Printf(
            TEXT("Menu: %s | Selected: %s"),
            *MenuName,
            *ItemName
        );
    }

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


void APauseMenu::HandlePauseAnnouncementFinished()
{
	if (!bPaused)
	{
		return;
	}

	CurrentNarrationComponent = nullptr;

	// Now announce the first highlighted option.
	PlayNavigationSound();
}


void APauseMenu::TogglePauseMenu()
{
	
	bPaused = !bPaused;

	// Reset menu state
	CurrentMenuIndex = 0;
	SelectedItemIndex = 0;
	MenuHistory.Empty();

	APlayerController* PC = GetWorld()->GetFirstPlayerController();

	if (bPaused)
	{
		if (PC)
		{
			EnableInput(PC);
		}

		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.0f);

		// Stop any previous narration.
		if (CurrentNarrationComponent)
		{
			CurrentNarrationComponent->OnAudioFinished.RemoveDynamic(
				this, &APauseMenu::HandlePauseAnnouncementFinished);

			CurrentNarrationComponent->Stop();
			CurrentNarrationComponent = nullptr;
		}

		// First announce that the game is paused.
		if (GamePausedSound)
		{
			CurrentNarrationComponent =
				UGameplayStatics::SpawnSound2D(this, GamePausedSound);

			if (CurrentNarrationComponent)
			{
				CurrentNarrationComponent->OnAudioFinished.AddDynamic(
					this, &APauseMenu::HandlePauseAnnouncementFinished);
			}
			else
			{
				// Fallback if the sound could not be created.
				PlayNavigationSound();
			}
		}
		else
		{
			// Fallback if no pause sound is assigned.
			PlayNavigationSound();
		}
	}
	else
	{
		// Stop any current narration
		if (CurrentNarrationComponent)
		{
			CurrentNarrationComponent->Stop();
			CurrentNarrationComponent = nullptr;
		}

		// Remove the settings widget if it is open
		if (IsValid(SettingsWidget))
		{
			SettingsWidget->RemoveFromParent();
			SettingsWidget = nullptr;
		}

		// Disable pause menu input and restore game input
		if (PC)
		{
			DisableInput(PC);
			PC->SetInputMode(FInputModeGameOnly());
			PC->bShowMouseCursor = false;
		}

		// Resume the game
		// Stop any currently playing sound
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);

		if (GEngine)
		{
			GEngine->RemoveOnScreenDebugMessage(4);
		}
	}
}

