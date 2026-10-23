// AudioSettingsSubsystem.h
// Holds the 5 volume values (0..1, 0.1 steps), applies them through a Sound Mix, and saves/loads them.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameFramework/SaveGame.h"
#include "Engine/DeveloperSettings.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "AudioSettingsSubsystem.generated.h"

UENUM(BlueprintType)
enum class EAudioChannel : uint8
{
	Master,
	SFX,
	Ambience,
	Music,
	Dialogue
};

/** Project Settings > Game > Audio Options : assign your Sound Mix and Sound Classes here. */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Audio Options"))
class PROD_GRUPP8_API UAudioOptionsSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** A Sound Mix asset (can be empty, the code adds the adjusters at runtime). */
	UPROPERTY(Config, EditAnywhere, Category = "Audio")
	TSoftObjectPtr<USoundMix> VolumeMix;

	/** One Sound Class per slider. Parent SFX/Ambience/Music/Dialogue under Master. */
	UPROPERTY(Config, EditAnywhere, Category = "Audio")
	TMap<EAudioChannel, TSoftObjectPtr<USoundClass>> SoundClasses;

	virtual FName GetCategoryName() const override { return TEXT("Game"); }
};

UCLASS()
class PROD_GRUPP8_API UAudioSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TMap<EAudioChannel, float> Volumes;
};

UCLASS()
class PROD_GRUPP8_API UAudioSettingsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static constexpr int32 NumSteps = 10; // 0.0, 0.1 ... 1.0

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, Category = "Audio Options")
	float GetVolume(EAudioChannel Channel) const;

	/** Snaps to 0.1 steps, clamps to 0..1 and applies immediately. */
	UFUNCTION(BlueprintCallable, Category = "Audio Options")
	void SetVolume(EAudioChannel Channel, float NewVolume);

	UFUNCTION(BlueprintCallable, Category = "Audio Options")
	void SaveSettings() const;

private:
	void OnWorldInit(UWorld* World, const UWorld::InitializationValues IVS);
	void ApplyChannel(UWorld* World, EAudioChannel Channel) const;
	void EnsureMixActive(UWorld* World, USoundMix* Mix) const;

	mutable TWeakObjectPtr<UWorld> MixPushedWorld;
	TMap<EAudioChannel, float> Volumes;
	FDelegateHandle WorldInitHandle;

	static const TCHAR* SlotName() { return TEXT("AudioOptions"); }
};