#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Containers/Ticker.h"
#include "SpeechTypes.h"
#include "SpeechSubsystem.generated.h"

class UAudioComponent;
class USoundClass;
class ISpeechBackend;
class FSpeechWorker;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpeechStarted, float, DurationSeconds);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSpeechFinished);

/**
 * Speech core.
 * Turns text into speech audio and plays it through Unreal.
 * Default engine: Piper (shipped with the game). Players can switch to Bengt etc. with SetVoice.
 * Menus, cutscene descriptions and gameplay announcements all go through here.
 * Recorded voice lines do NOT go through here; they are normal sound assets.
 */
UCLASS()
class AUDIOGAMESPEECH_API USpeechSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ---------- Speaking ----------

	// Interrupt = cut off current speech (menus). Otherwise queue after it.
	UFUNCTION(BlueprintCallable, Category = "Speech")
	void Speak(const FText& Text, bool bInterrupt = true);

	// Stop now and clear the queue.
	UFUNCTION(BlueprintCallable, Category = "Speech")
	void StopSpeaking();

	UFUNCTION(BlueprintPure, Category = "Speech")
	bool IsSpeaking() const;

	// Say the last line again.
	UFUNCTION(BlueprintCallable, Category = "Speech")
	void RepeatLast();

	// Render lines ahead of time so they play instantly. Call for menus at startup.
	// Re-rendered automatically when voice or speed changes.
	UFUNCTION(BlueprintCallable, Category = "Speech")
	void Precache(const TArray<FText>& Lines);

	// ---------- Voices ----------

	UFUNCTION(BlueprintCallable, Category = "Speech|Voice")
	TArray<FSpeechVoice> GetVoices(bool bSwedishOnly = true) const;

	// Also switches engine if the voice belongs to another one (the old engine shuts down).
	UFUNCTION(BlueprintCallable, Category = "Speech|Voice")
	bool SetVoice(const FString& VoiceId);

	UFUNCTION(BlueprintPure, Category = "Speech|Voice")
	FSpeechVoice GetCurrentVoice() const { return CurrentVoice; }

	// False = no Swedish voice installed. Play the recorded help message.
	UFUNCTION(BlueprintPure, Category = "Speech|Voice")
	bool HasSwedishVoice() const;

	// ---------- Settings ----------

	// 1.0 = normal. Range 0.5 to 6.0.
	UFUNCTION(BlueprintCallable, Category = "Speech|Settings")
	void SetSpeakingRate(float NewRate);

	UFUNCTION(BlueprintPure, Category = "Speech|Settings")
	float GetSpeakingRate() const { return SpeakingRate; }

	// 0 to 1.
	UFUNCTION(BlueprintCallable, Category = "Speech|Settings")
	void SetVolume(float NewVolume);

	// Assign your "Speech" Sound Class so the volume slider and ducking apply.
	UFUNCTION(BlueprintCallable, Category = "Speech|Settings")
	void SetSoundClass(USoundClass* InSoundClass);

	// ---------- Events ----------

	// Fires when a line starts. Duration is exact (use it for cutscene timing).
	UPROPERTY(BlueprintAssignable, Category = "Speech")
	FOnSpeechStarted OnSpeechStarted;

	// Fires when a line ends normally. Not fired on interrupt.
	UPROPERTY(BlueprintAssignable, Category = "Speech")
	FOnSpeechFinished OnSpeechFinished;

private:
	void SelectDefaultVoice();
	void StartWorker();
	void PrepareEngine();
	void PlayNext();
	void RequestClip(const FString& Text, bool bPlayWhenReady);
	void HandleClipReady(const FString& Key, bool bOk, const FSpeechClip& Clip, bool bPlay);
	void PlayClip(const FSpeechClip& Clip);
	bool HandleClipTimer(float DeltaTime);
	void FinishCurrentLine();
	void OnSettingsChanged();
	void AddToCache(const FString& Key, const FSpeechClip& Clip);
	FString GetSettingsPrefix() const;
	FString MakeCacheKey(const FString& Text) const;

	// All engines, and a background thread for the active one (CurrentVoice.Engine)
	TMap<ESpeechEngine, TSharedPtr<ISpeechBackend>> Backends;
	TSharedPtr<FSpeechWorker> Worker;

	// Voices and settings
	TArray<FSpeechVoice> AvailableVoices;
	FSpeechVoice CurrentVoice;
	float SpeakingRate = 1.f;
	float Volume = 1.f;

	UPROPERTY()
	TObjectPtr<USoundClass> SoundClass;

	UPROPERTY()
	TObjectPtr<UAudioComponent> ActiveComponent;

	// Cache: settings + text -> audio
	TMap<FString, FSpeechClip> ClipCache;
	int64 CacheBytes = 0;
	TArray<FString> PrecachedLines;

	// Playback state
	TArray<FString> Queue; // TODO next step: priority queue
	FString LastSpoken;
	bool bBusy = false;    // waiting for or playing a line
	bool bShuttingDown = false;

	FCancelFlag SpeechCancel;
	FCancelFlag PrecacheCancel;
	FTSTicker::FDelegateHandle FinishHandle;
};
