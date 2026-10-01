#include "SpeechSubsystem.h"
#include "SpeechLog.h"
#include "SpeechBackend.h"
#include "SpeechWorker.h"
#include "PiperSpeechBackend.h"
#include "WinRTSpeechBackend.h"

#include "Async/Async.h"
#include "Components/AudioComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundWaveProcedural.h"

namespace
{
	constexpr float MinRate = 0.5f;
	constexpr float MaxRate = 6.0f;
	constexpr float EndPadding = 0.05f;                     // small gap after each line
	constexpr int64 MaxCacheBytes = 64ll * 1024 * 1024;     // ~25 min of speech

	bool IsSwedish(const FSpeechVoice& Voice)
	{
		return Voice.Language.StartsWith(TEXT("sv"), ESearchCase::IgnoreCase);
	}

	FCancelFlag MakeFlag()
	{
		return MakeShared<std::atomic<bool>, ESPMode::ThreadSafe>(false);
	}
}

// ---------- Lifetime ----------

void USpeechSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	SpeechCancel = MakeFlag();
	PrecacheCancel = MakeFlag();

	// Built in: works on download
	const FString PiperDir = FPiperSpeechBackend::FindPiperDir();
	if (!PiperDir.IsEmpty())
	{
		Backends.Add(ESpeechEngine::Piper, MakeShared<FPiperSpeechBackend>(PiperDir));
	}
	else
	{
		UE_LOG(LogAudioGameSpeech, Warning, TEXT("Piper not found. Run Plugins/AudioGameSpeech/ThirdParty/Piper/Setup-Piper.ps1"));
	}

#if WITH_WINRT_SPEECH
	Backends.Add(ESpeechEngine::WindowsOneCore, MakeShared<FWinRTSpeechBackend>());
#endif

	// Implement third party voice support here later:
	// Backends.Add(ESpeechEngine::ThirdPartySAPI5, MakeShared<FSapi5SpeechBackend>());

	for (const TPair<ESpeechEngine, TSharedPtr<ISpeechBackend>>& Pair : Backends)
	{
		AvailableVoices.Append(Pair.Value->GetVoices());
	}

	SelectDefaultVoice();
	StartWorker();
	PrepareEngine();

	UE_LOG(LogAudioGameSpeech, Log, TEXT("Speech ready. Voice: %s (%s). Swedish voice installed: %s"),
		*CurrentVoice.DisplayName, *CurrentVoice.Language, HasSwedishVoice() ? TEXT("yes") : TEXT("NO"));
}

void USpeechSubsystem::Deinitialize()
{
	StopSpeaking();
	if (PrecacheCancel.IsValid())
	{
		PrecacheCancel->store(true);
	}
	bShuttingDown = true;

	Worker.Reset();   // joins the thread, active engine shuts down
	Backends.Empty();
	ClipCache.Empty();

	Super::Deinitialize();
}

void USpeechSubsystem::SelectDefaultVoice()
{
	// 1. Piper (built in)  2. Bengt  3. any Swedish voice  4. anything (game should play recorded help message)
	const FSpeechVoice* Pick = AvailableVoices.FindByPredicate([](const FSpeechVoice& V) { return V.Engine == ESpeechEngine::Piper && IsSwedish(V); });
	if (!Pick)
	{
		Pick = AvailableVoices.FindByPredicate([](const FSpeechVoice& V) { return V.DisplayName.Contains(TEXT("Bengt")); });
	}
	if (!Pick)
	{
		Pick = AvailableVoices.FindByPredicate(IsSwedish);
	}
	if (!Pick && AvailableVoices.Num() > 0)
	{
		Pick = &AvailableVoices[0];
	}
	if (Pick)
	{
		CurrentVoice = *Pick;
	}
}

void USpeechSubsystem::StartWorker()
{
	// Joins the old thread, so its engine shuts down (e.g. piper.exe closes)
	Worker.Reset();

	const TSharedPtr<ISpeechBackend>* Backend = Backends.Find(CurrentVoice.Engine);
	if (Backend && Backend->IsValid())
	{
		Worker = MakeShared<FSpeechWorker>(Backend->ToSharedRef());
	}
}

void USpeechSubsystem::PrepareEngine()
{
	if (!Worker.IsValid())
	{
		return;
	}
	// Piper loads its model here (~1 s) instead of on the first line
	FSpeechJob Job;
	Job.VoiceId = CurrentVoice.Id;
	Job.Rate = SpeakingRate;
	Job.bPrepareOnly = true;
	Worker->Enqueue(MoveTemp(Job));
}

// ---------- Speaking ----------

void USpeechSubsystem::Speak(const FText& Text, bool bInterrupt)
{
	FString Line = Text.ToString().TrimStartAndEnd();
	if (Line.IsEmpty())
	{
		return;
	}

	if (bInterrupt)
	{
		StopSpeaking();
	}

	// TODO next step: priority queue + rule for outdated lines (decide later)
	Queue.Add(MoveTemp(Line));
	PlayNext();
}

void USpeechSubsystem::StopSpeaking()
{
	// Cancel lines still being rendered
	if (SpeechCancel.IsValid())
	{
		SpeechCancel->store(true);
	}
	SpeechCancel = MakeFlag();

	Queue.Reset();

	if (FinishHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(FinishHandle);
		FinishHandle.Reset();
	}

	if (IsValid(ActiveComponent))
	{
		ActiveComponent->Stop();
	}
	ActiveComponent = nullptr;
	bBusy = false;
}

bool USpeechSubsystem::IsSpeaking() const
{
	return bBusy || Queue.Num() > 0;
}

void USpeechSubsystem::RepeatLast()
{
	if (LastSpoken.IsEmpty())
	{
		return;
	}
	const FString Line = LastSpoken;
	StopSpeaking();
	Queue.Add(Line);
	PlayNext();
}

void USpeechSubsystem::Precache(const TArray<FText>& Lines)
{
	for (const FText& Text : Lines)
	{
		const FString Line = Text.ToString().TrimStartAndEnd();
		if (Line.IsEmpty())
		{
			continue;
		}
		PrecachedLines.AddUnique(Line);
		if (!ClipCache.Contains(MakeCacheKey(Line)))
		{
			RequestClip(Line, false);
		}
	}
}

// ---------- Playback flow ----------

void USpeechSubsystem::PlayNext()
{
	if (bBusy || Queue.Num() == 0)
	{
		return;
	}

	const FString Line = Queue[0];
	Queue.RemoveAt(0);
	LastSpoken = Line;
	bBusy = true;

	// Already rendered: play instantly
	if (const FSpeechClip* Cached = ClipCache.Find(MakeCacheKey(Line)))
	{
		PlayClip(*Cached);
		return;
	}

	RequestClip(Line, true);
}

void USpeechSubsystem::RequestClip(const FString& Text, bool bPlayWhenReady)
{
	if (!Worker.IsValid())
	{
		if (bPlayWhenReady)
		{
			FinishCurrentLine(); // no engine: skip line so nothing hangs
		}
		return;
	}

	FSpeechJob Job;
	Job.Text = Text;
	Job.VoiceId = CurrentVoice.Id;
	Job.Rate = SpeakingRate;
	Job.Cancelled = bPlayWhenReady ? SpeechCancel : PrecacheCancel;

	// Implement third party voice support here later: Job.Engine = CurrentVoice.Engine;

	const FString Key = MakeCacheKey(Text);
	const FCancelFlag Flag = Job.Cancelled;
	TWeakObjectPtr<USpeechSubsystem> WeakThis(this);

	Job.OnDone = [WeakThis, Key, Flag, bPlayWhenReady](bool bOk, FSpeechClip&& Clip)
	{
		// Worker thread -> game thread
		AsyncTask(ENamedThreads::GameThread, [WeakThis, Key, Flag, bPlayWhenReady, bOk, Clip = MoveTemp(Clip)]()
		{
			USpeechSubsystem* Self = WeakThis.Get();
			if (!Self || Self->bShuttingDown)
			{
				return;
			}
			const bool bStillWanted = bPlayWhenReady && !Flag->load();
			Self->HandleClipReady(Key, bOk, Clip, bStillWanted);
		});
	};

	Worker->Enqueue(MoveTemp(Job));
}

void USpeechSubsystem::HandleClipReady(const FString& Key, bool bOk, const FSpeechClip& Clip, bool bPlay)
{
	// Cache only if voice/speed haven't changed since the request
	if (bOk && Key.StartsWith(GetSettingsPrefix(), ESearchCase::CaseSensitive))
	{
		AddToCache(Key, Clip);
	}

	if (!bPlay)
	{
		return;
	}

	if (bOk)
	{
		PlayClip(Clip);
	}
	else
	{
		UE_LOG(LogAudioGameSpeech, Warning, TEXT("Could not render line, skipping."));
		FinishCurrentLine();
	}
}

void USpeechSubsystem::PlayClip(const FSpeechClip& Clip)
{
	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World || Clip.PcmData.Num() == 0)
	{
		FinishCurrentLine();
		return;
	}

	// PCM -> playable sound
	USoundWaveProcedural* Wave = NewObject<USoundWaveProcedural>(this);
	Wave->SetSampleRate(Clip.SampleRate);
	Wave->NumChannels = Clip.NumChannels;
	Wave->Duration = Clip.Duration;
	Wave->bLooping = false;
	Wave->SoundClassObject = SoundClass; // volume slider / ducking
	Wave->QueueAudio(Clip.PcmData.GetData(), Clip.PcmData.Num());

	ActiveComponent = UGameplayStatics::CreateSound2D(World, Wave, Volume, 1.f, 0.f, nullptr,
		/*bPersistAcrossLevelTransition*/ true, /*bAutoDestroy*/ true);
	if (!ActiveComponent)
	{
		FinishCurrentLine();
		return;
	}

	ActiveComponent->bIsUISound = true; // keeps talking while the game is paused
	ActiveComponent->Play();

	// Procedural sounds never end by themselves: stop after the exact length
	FinishHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &USpeechSubsystem::HandleClipTimer),
		Clip.Duration + EndPadding);

	// Last: listeners may call Speak() from here
	OnSpeechStarted.Broadcast(Clip.Duration);
}

bool USpeechSubsystem::HandleClipTimer(float DeltaTime)
{
	FinishHandle.Reset();
	FinishCurrentLine();
	return false; // run once
}

void USpeechSubsystem::FinishCurrentLine()
{
	if (IsValid(ActiveComponent))
	{
		ActiveComponent->Stop();
	}
	ActiveComponent = nullptr;
	bBusy = false;

	OnSpeechFinished.Broadcast();
	PlayNext(); // does nothing if a listener already started a new line
}

// ---------- Voices ----------

TArray<FSpeechVoice> USpeechSubsystem::GetVoices(bool bSwedishOnly) const
{
	if (!bSwedishOnly)
	{
		return AvailableVoices;
	}
	return AvailableVoices.FilterByPredicate(IsSwedish);
}

bool USpeechSubsystem::SetVoice(const FString& VoiceId)
{
	const FSpeechVoice* Found = AvailableVoices.FindByPredicate([&VoiceId](const FSpeechVoice& V) { return V.Id == VoiceId; });
	if (!Found)
	{
		return false;
	}
	if (Found->Id == CurrentVoice.Id)
	{
		return true;
	}

	const bool bEngineChanged = Found->Engine != CurrentVoice.Engine;
	CurrentVoice = *Found;
	if (bEngineChanged)
	{
		// The old worker drops its queued jobs, so stop first (nothing waits on them)
		StopSpeaking();
		StartWorker();
	}
	OnSettingsChanged();
	return true;
}

bool USpeechSubsystem::HasSwedishVoice() const
{
	return AvailableVoices.ContainsByPredicate(IsSwedish);
}

// ---------- Settings ----------

void USpeechSubsystem::SetSpeakingRate(float NewRate)
{
	NewRate = FMath::Clamp(NewRate, MinRate, MaxRate);
	if (FMath::IsNearlyEqual(NewRate, SpeakingRate, 0.01f))
	{
		return;
	}
	SpeakingRate = NewRate;
	OnSettingsChanged();
}

void USpeechSubsystem::SetVolume(float NewVolume)
{
	Volume = FMath::Clamp(NewVolume, 0.f, 1.f);
	if (IsValid(ActiveComponent))
	{
		ActiveComponent->SetVolumeMultiplier(Volume);
	}
}

void USpeechSubsystem::SetSoundClass(USoundClass* InSoundClass)
{
	SoundClass = InSoundClass;
}

void USpeechSubsystem::OnSettingsChanged()
{
	// Old audio is wrong voice/speed: drop it
	ClipCache.Empty();
	CacheBytes = 0;

	PrepareEngine();

	// Cancel old precache jobs, re-render menu lines
	PrecacheCancel->store(true);
	PrecacheCancel = MakeFlag();
	for (const FString& Line : PrecachedLines)
	{
		RequestClip(Line, false);
	}
}

// ---------- Cache ----------

void USpeechSubsystem::AddToCache(const FString& Key, const FSpeechClip& Clip)
{
	if (ClipCache.Contains(Key))
	{
		return;
	}
	// Simple cap: start over when full. Lines re-render on demand.
	if (CacheBytes + Clip.PcmData.Num() > MaxCacheBytes)
	{
		ClipCache.Empty();
		CacheBytes = 0;
	}
	ClipCache.Add(Key, Clip);
	CacheBytes += Clip.PcmData.Num();
}

FString USpeechSubsystem::GetSettingsPrefix() const
{
	return FString::Printf(TEXT("%s|%.2f|"), *CurrentVoice.Id, SpeakingRate);
}

FString USpeechSubsystem::MakeCacheKey(const FString& Text) const
{
	return GetSettingsPrefix() + Text;
}
