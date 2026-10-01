#include "SpeechTestActor.h"
#include "SpeechSubsystem.h"
#include "SpeechLog.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"

ASpeechTestActor::ASpeechTestActor()
{
	PrimaryActorTick.bCanEverTick = false;

	TestLine = NSLOCTEXT("SpeechTest", "TestLine", "Hej! Talsystemet fungerar.");
	QueuedLine = NSLOCTEXT("SpeechTest", "QueuedLine", "Det här är nästa rad i kön.");
}

void ASpeechTestActor::BeginPlay()
{
	Super::BeginPlay();

	USpeechSubsystem* Speech = GetSpeech();
	if (!Speech)
	{
		Report(TEXT("Speech test: no SpeechSubsystem (is the plugin enabled?)"), FColor::Red);
		return;
	}

	const TArray<FSpeechVoice> Voices = Speech->GetVoices(false);
	Report(FString::Printf(TEXT("Speech test: %d voices installed"), Voices.Num()));
	for (const FSpeechVoice& Voice : Voices)
	{
		UE_LOG(LogAudioGameSpeech, Log, TEXT("  Voice: %s (%s)"), *Voice.DisplayName, *Voice.Language);
	}

	const FSpeechVoice Current = Speech->GetCurrentVoice();
	Report(FString::Printf(TEXT("Speech test: using %s (%s). Swedish voice: %s"),
		*Current.DisplayName, *Current.Language, Speech->HasSwedishVoice() ? TEXT("yes") : TEXT("NO")),
		Speech->HasSwedishVoice() ? FColor::Green : FColor::Orange);

	Speech->OnSpeechStarted.AddDynamic(this, &ASpeechTestActor::HandleSpeechStarted);
	Speech->OnSpeechFinished.AddDynamic(this, &ASpeechTestActor::HandleSpeechFinished);

	RunTest();
}

void ASpeechTestActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (USpeechSubsystem* Speech = GetSpeech())
	{
		Speech->OnSpeechStarted.RemoveDynamic(this, &ASpeechTestActor::HandleSpeechStarted);
		Speech->OnSpeechFinished.RemoveDynamic(this, &ASpeechTestActor::HandleSpeechFinished);
	}
	Super::EndPlay(EndPlayReason);
}

void ASpeechTestActor::RunTest()
{
	USpeechSubsystem* Speech = GetSpeech();
	if (!Speech)
	{
		Report(TEXT("Speech test: only works while playing (PIE)."), FColor::Orange);
		return;
	}

	Speech->SetSpeakingRate(SpeakingRate);
	Speech->Speak(TestLine, /*bInterrupt*/ true);
	Speech->Speak(QueuedLine, /*bInterrupt*/ false);
}

void ASpeechTestActor::HandleSpeechStarted(float DurationSeconds)
{
	Report(FString::Printf(TEXT("Speech started (%.2f s)"), DurationSeconds));
}

void ASpeechTestActor::HandleSpeechFinished()
{
	Report(TEXT("Speech finished"));
}

USpeechSubsystem* ASpeechTestActor::GetSpeech() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<USpeechSubsystem>() : nullptr;
}

void ASpeechTestActor::Report(const FString& Message, const FColor& Color) const
{
	UE_LOG(LogAudioGameSpeech, Log, TEXT("%s"), *Message);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 8.f, Color, Message);
	}
}
