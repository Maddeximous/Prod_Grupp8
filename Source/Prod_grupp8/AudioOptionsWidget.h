// AudioOptionsWidget.h
// Parent class for your WBP_AudioOptions widget blueprint.
// Replace YOURGAME_API with your module's API macro.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AudioSettingsSubsystem.h"
#include "AudioOptionsWidget.generated.h"

class USlider;
class UButton;
class USoundBase;
class UAudioComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAudioOptionsBack);

UCLASS(Abstract)
class PROD_GRUPP8_API UAudioOptionsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * 11 spoken cues, index 0 = "zero" (0.0), index 1 = "one" (0.1) ... index 10 = "ten" (1.0).
	 * Played every time a slider lands on a new value.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio Options|Cues")
	TArray<TObjectPtr<USoundBase>> ValueCues;

	/** Optional: spoken slider name ("Master", "Effects"...), played when a slider gets focus. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio Options|Cues")
	TMap<EAudioChannel, TObjectPtr<USoundBase>> ChannelNameCues;

	/** Optional: spoken "Back" cue, played when the Back button gets focus. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio Options|Cues")
	TObjectPtr<USoundBase> BackCue;

	/** Your main menu widget class. If set, Back closes this menu and opens that one. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio Options|Navigation")
	TSubclassOf<UUserWidget> MainMenuClass;

	/** Fired when Back is pressed (button, Escape or gamepad B). Use it if you'd rather handle navigation in Blueprint. */
	UPROPERTY(BlueprintAssignable, Category = "Audio Options|Navigation")
	FOnAudioOptionsBack OnBackRequested;

protected:
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// Names must match the widgets in the Designer exactly.
	UPROPERTY(meta = (BindWidget)) TObjectPtr<USlider> Slider_Master;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<USlider> Slider_SFX;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<USlider> Slider_Ambience;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<USlider> Slider_Music;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<USlider> Slider_Dialogue;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> Button_Back;

private:
	UFUNCTION() void OnBackClicked();
	void GoBack();
	bool bBackFocused = false;

	UFUNCTION() void OnMasterChanged(float Value);
	UFUNCTION() void OnSFXChanged(float Value);
	UFUNCTION() void OnAmbienceChanged(float Value);
	UFUNCTION() void OnMusicChanged(float Value);
	UFUNCTION() void OnDialogueChanged(float Value);

	void HandleChanged(EAudioChannel Channel, float Value);
	void PlayCue(USoundBase* Cue);
	USlider* GetSlider(EAudioChannel Channel) const;
	void ConfigureSlider(USlider* Slider) const;

	UAudioSettingsSubsystem* GetSettings() const;

	TMap<EAudioChannel, int32> LastStep;
	TWeakObjectPtr<UAudioComponent> CurrentCue;
	bool bHasFocusedChannel = false;
	EAudioChannel FocusedChannel = EAudioChannel::Master;
};