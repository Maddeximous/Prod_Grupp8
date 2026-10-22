// Fill out your copyright notice in the Description page of Project Settings.


#include "PauseMenu.h"
#include "Inventory.h"
#include "Fish.h"
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

void APauseMenu::MoveUp()
{
	if (bBrowsingFishInventory)
	{
		BrowseFishUp();
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
	
	VoiceState();
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
	if (bBrowsingFishInventory)
	{
		BrowseFishDown();
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
			VoiceState();
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
		bPaused = false;
		break;

	case EPauseMenuAction::Restart:
		break;

	case EPauseMenuAction::Quit:
		break;

	case EPauseMenuAction::OpenSettings:
		break;

	case EPauseMenuAction::None:
		break;
	}
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
    FishMenu.Items.Empty();

    for (AFish* Fish : Inventory->Fish)
    {
        if (!IsValid(Fish))
            continue;

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

    UE_LOG(LogTemp, Warning, TEXT("Fish menu populated: %d items"),
        FishMenu.Items.Num());

    CurrentMenuIndex = 2;
    SelectedItemIndex = 0;

    VoiceState();
}

void APauseMenu::GoBack()
{
	if (bBrowsingFishInventory)
	{
		bBrowsingFishInventory = false;
		FishList.Empty();
		CurrentFishIndex = 0;

		CurrentMenuIndex = MenuHistory.Last();
		MenuHistory.Pop();

		SelectedItemIndex = 0;
		VoiceState();
		return;
	}
	
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
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.0f);
	}
	else
	{
		DisableInput(GetWorld()->GetFirstPlayerController());
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);

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