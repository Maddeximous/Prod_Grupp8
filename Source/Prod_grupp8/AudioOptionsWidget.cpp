// AudioOptionsWidget.cpp
#include "AudioOptionsWidget.h"
#include "Components/Slider.h"
#include "Components/Button.h"
#include "InputCoreTypes.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Widget.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "Framework/Application/SlateApplication.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "Sound/SoundBase.h"

namespace
{
	const EAudioChannel AllChannels[] = {
		EAudioChannel::Master, EAudioChannel::SFX, EAudioChannel::Ambience,
		EAudioChannel::Music, EAudioChannel::Dialogue };
}

UAudioSettingsSubsystem* UAudioOptionsWidget::GetSettings() const
{
	const UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UAudioSettingsSubsystem>() : nullptr;
}

USlider* UAudioOptionsWidget::GetSlider(EAudioChannel Channel) const
{
	switch (Channel)
	{
	case EAudioChannel::Master:   return Slider_Master;
	case EAudioChannel::SFX:      return Slider_SFX;
	case EAudioChannel::Ambience: return Slider_Ambience;
	case EAudioChannel::Music:    return Slider_Music;
	case EAudioChannel::Dialogue: return Slider_Dialogue;
	}
	return nullptr;
}

void UAudioOptionsWidget::ConfigureSlider(USlider* Slider) const
{
	Slider->SetMinValue(0.0f);
	Slider->SetMaxValue(1.0f);
	Slider->SetStepSize(0.1f);
	Slider->SetLocked(false);
}

void UAudioOptionsWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	for (EAudioChannel Channel : AllChannels)
	{
		ConfigureSlider(GetSlider(Channel));
	}

	Slider_Master->OnValueChanged.AddDynamic(this, &UAudioOptionsWidget::OnMasterChanged);
	Slider_SFX->OnValueChanged.AddDynamic(this, &UAudioOptionsWidget::OnSFXChanged);
	Slider_Ambience->OnValueChanged.AddDynamic(this, &UAudioOptionsWidget::OnAmbienceChanged);
	Slider_Music->OnValueChanged.AddDynamic(this, &UAudioOptionsWidget::OnMusicChanged);
	Slider_Dialogue->OnValueChanged.AddDynamic(this, &UAudioOptionsWidget::OnDialogueChanged);

	Button_Back->OnClicked.AddDynamic(this, &UAudioOptionsWidget::OnBackClicked);

	// Keep controller navigation inside this menu so it can't jump to the main menu buttons underneath.
	Slider_Master->SetNavigationRuleBase(EUINavigation::Up, EUINavigationRule::Stop);
	Button_Back->SetNavigationRuleBase(EUINavigation::Down, EUINavigationRule::Stop);
	Button_Back->SetNavigationRuleBase(EUINavigation::Left, EUINavigationRule::Stop);
	Button_Back->SetNavigationRuleBase(EUINavigation::Right, EUINavigationRule::Stop);
	// On the sliders, Left/Right navigation steps the value by 0.1 instead of moving focus.
	FCustomWidgetNavigationDelegate LeftDelegate;
	LeftDelegate.BindDynamic(this, &UAudioOptionsWidget::OnNavLeft);
	FCustomWidgetNavigationDelegate RightDelegate;
	RightDelegate.BindDynamic(this, &UAudioOptionsWidget::OnNavRight);
	for (EAudioChannel Channel : AllChannels)
	{
		GetSlider(Channel)->SetNavigationRuleCustom(EUINavigation::Left, LeftDelegate);
		GetSlider(Channel)->SetNavigationRuleCustom(EUINavigation::Right, RightDelegate);
	}
}

FReply UAudioOptionsWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (Key == EKeys::Escape || Key == EKeys::Gamepad_FaceButton_Right)
	{
		GoBack();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UAudioOptionsWidget::OnBackClicked()
{
	GoBack();
}

void UAudioOptionsWidget::GoBack()
{
	if (const UAudioSettingsSubsystem* Settings = GetSettings())
	{
		Settings->SaveSettings();
	}
	if (CurrentCue.IsValid())
	{
		CurrentCue->Stop();
	}

	OnBackRequested.Broadcast();

	if (MainMenuClass)
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (UUserWidget* MainMenu = CreateWidget<UUserWidget>(PC, MainMenuClass))
			{
				MainMenu->AddToViewport();

				// Hand input focus to the main menu so the controller works right away.
				FInputModeUIOnly InputMode;
				InputMode.SetWidgetToFocus(MainMenu->TakeWidget());
				InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				PC->SetInputMode(InputMode);

				// Focus the chosen button one tick later, once the new widget is fully built.
				TWeakObjectPtr<UUserWidget> WeakMenu(MainMenu);
				TWeakObjectPtr<APlayerController> WeakPC(PC);
				const FName FocusName = MainMenuFocusWidgetName;
				PC->GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([WeakMenu, WeakPC, FocusName]()
				{
					if (!WeakMenu.IsValid() || !WeakPC.IsValid())
					{
						return;
					}
					UWidget* Target = nullptr;
					if (!FocusName.IsNone() && WeakMenu->WidgetTree)
					{
						Target = WeakMenu->WidgetTree->FindWidget(FocusName);
					}
					if (Target)
					{
						Target->SetUserFocus(WeakPC.Get());
					}
					else
					{
						WeakMenu->SetUserFocus(WeakPC.Get());
					}
				}));
			}
		}
		RemoveFromParent();
	}
	else if (!MainMenuLevelName.IsNone())
	{
		UGameplayStatics::OpenLevel(this, MainMenuLevelName);
	}
	else
	{
		// Default case: the main menu is still on screen underneath this widget.
		// Just close the options menu and hand controller focus back to the button that opened it.
		const TWeakPtr<SWidget> FocusTarget = PreviousFocusWidget;
		APlayerController* PC = GetOwningPlayer();
		RemoveFromParent();

		if (PC)
		{
			PC->GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([FocusTarget]()
			{
				if (const TSharedPtr<SWidget> Target = FocusTarget.Pin())
				{
					FSlateApplication::Get().SetUserFocus(0, Target, EFocusCause::SetDirectly);
				}
			}));
		}
	}
}

void UAudioOptionsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Remember what had focus (your main menu's Options button) before we steal it, so Back can give it back.
	PreviousFocusWidget = FSlateApplication::Get().GetUserFocusedWidget(0);
	FocusGraceTime = 1.0f;

	// Show the stored values without triggering cues.
	if (UAudioSettingsSubsystem* Settings = GetSettings())
	{
		for (EAudioChannel Channel : AllChannels)
		{
			const float Value = Settings->GetVolume(Channel);
			GetSlider(Channel)->SetValue(Value);
			LastStep.Add(Channel, FMath::RoundToInt(Value * UAudioSettingsSubsystem::NumSteps));
		}
	}

	// Keyboard / gamepad users start on the first slider (Up/Down moves between sliders, Left/Right changes value).
	Slider_Master->SetKeyboardFocus();
}

void UAudioOptionsWidget::NativeDestruct()
{
	if (CurrentCue.IsValid())
	{
		CurrentCue->Stop();
	}
	if (const UAudioSettingsSubsystem* Settings = GetSettings())
	{
		Settings->SaveSettings();
	}
	Super::NativeDestruct();
}

void UAudioOptionsWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Right after opening, the code that created this widget may move focus onto the widget itself
	// (e.g. Set Input Mode UI Only with this widget as target), which leaves the sliders without focus.
	// For a short time, put focus back on the first slider whenever nothing in this menu has it.
	if (FocusGraceTime > 0.f)
	{
		FocusGraceTime -= InDeltaTime;

		bool bAnyFocus = Button_Back && Button_Back->HasAnyUserFocus();
		for (EAudioChannel Channel : AllChannels)
		{
			const USlider* Slider = GetSlider(Channel);
			if (Slider && Slider->HasAnyUserFocus())
			{
				bAnyFocus = true;
			}
		}
		if (!bAnyFocus)
		{
			Slider_Master->SetKeyboardFocus();
		}
	}

	// Announce the Back button when it gets focus (optional, only if BackCue is assigned).
	if (Button_Back && Button_Back->HasAnyUserFocus())
	{
		if (!bBackFocused)
		{
			bBackFocused = true;
			bHasFocusedChannel = false;
			PlayCue(BackCue);
		}
		return;
	}
	bBackFocused = false;

	// Announce which slider has focus (optional, only if a name cue is assigned).
	for (EAudioChannel Channel : AllChannels)
	{
		const USlider* Slider = GetSlider(Channel);
		if (Slider && Slider->HasAnyUserFocus())
		{
			if (!bHasFocusedChannel || FocusedChannel != Channel)
			{
				bHasFocusedChannel = true;
				FocusedChannel = Channel;
				if (const TObjectPtr<USoundBase>* NameCue = ChannelNameCues.Find(Channel))
				{
					PlayCue(*NameCue);
				}
			}
			return;
		}
	}
}

void UAudioOptionsWidget::OnMasterChanged(float Value)   { HandleChanged(EAudioChannel::Master, Value); }
void UAudioOptionsWidget::OnSFXChanged(float Value)      { HandleChanged(EAudioChannel::SFX, Value); }
void UAudioOptionsWidget::OnAmbienceChanged(float Value) { HandleChanged(EAudioChannel::Ambience, Value); }
void UAudioOptionsWidget::OnMusicChanged(float Value)    { HandleChanged(EAudioChannel::Music, Value); }
void UAudioOptionsWidget::OnDialogueChanged(float Value) { HandleChanged(EAudioChannel::Dialogue, Value); }

UWidget* UAudioOptionsWidget::OnNavLeft(EUINavigation NavDirection)  { return StepFocusedSlider(-1); }
UWidget* UAudioOptionsWidget::OnNavRight(EUINavigation NavDirection) { return StepFocusedSlider(+1); }

UWidget* UAudioOptionsWidget::StepFocusedSlider(int32 Direction)
{
	for (EAudioChannel Channel : AllChannels)
	{
		USlider* Slider = GetSlider(Channel);
		if (Slider && Slider->HasAnyUserFocus())
		{
			const int32 NewStep = FMath::Clamp(
				FMath::RoundToInt(Slider->GetValue() * UAudioSettingsSubsystem::NumSteps) + Direction,
				0, UAudioSettingsSubsystem::NumSteps);
			const float NewValue = NewStep / static_cast<float>(UAudioSettingsSubsystem::NumSteps);

			if (!FMath::IsNearlyEqual(Slider->GetValue(), NewValue))
			{
				Slider->SetValue(NewValue);
				HandleChanged(Channel, NewValue);
			}
			return Slider; // keep focus on the same slider
		}
	}
	return nullptr;
}

void UAudioOptionsWidget::HandleChanged(EAudioChannel Channel, float Value)
{
	const int32 Step = FMath::Clamp(FMath::RoundToInt(Value * UAudioSettingsSubsystem::NumSteps), 0, UAudioSettingsSubsystem::NumSteps);
	const float Snapped = Step / static_cast<float>(UAudioSettingsSubsystem::NumSteps);

	// Force exact 0.1 increments even when dragging with the mouse.
	if (USlider* Slider = GetSlider(Channel))
	{
		if (!FMath::IsNearlyEqual(Slider->GetValue(), Snapped))
		{
			Slider->SetValue(Snapped);
		}
	}

	// Only react when we actually land on a different step.
	const int32* Previous = LastStep.Find(Channel);
	if (Previous && *Previous == Step)
	{
		return;
	}
	LastStep.Add(Channel, Step);

	if (UAudioSettingsSubsystem* Settings = GetSettings())
	{
		Settings->SetVolume(Channel, Snapped);
	}

	if (ValueCues.IsValidIndex(Step))
	{
		PlayCue(ValueCues[Step]);
	}
}

void UAudioOptionsWidget::PlayCue(USoundBase* Cue)
{
	if (!Cue)
	{
		return;
	}

	// Cut the previous cue so fast changes never pile up into a mush of voices.
	if (CurrentCue.IsValid())
	{
		CurrentCue->Stop();
	}
	CurrentCue = UGameplayStatics::SpawnSound2D(this, Cue);
}