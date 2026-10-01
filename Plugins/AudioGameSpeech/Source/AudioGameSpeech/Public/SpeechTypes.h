#pragma once

#include "CoreMinimal.h"
#include <atomic>
#include "SpeechTypes.generated.h"

// Which speech system a voice comes from.
UENUM(BlueprintType)
enum class ESpeechEngine : uint8
{
	WindowsOneCore  UMETA(DisplayName = "Windows (OneCore)"),

	// Implement third party voice support here later (SAPI5 voices, e.g. Acapela, Vocalizer).
	// Listed now so saved settings stay valid. Not implemented yet.
	ThirdPartySAPI5 UMETA(DisplayName = "Third-party (SAPI5)"),

	// Offline voice shipped with the game. Default, works without any Windows setup.
	Piper           UMETA(DisplayName = "Piper (built in)")
};

// A voice the player can pick.
USTRUCT(BlueprintType)
struct FSpeechVoice
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Speech")
	FString Id;

	UPROPERTY(BlueprintReadOnly, Category = "Speech")
	FString DisplayName;

	// e.g. "sv-SE"
	UPROPERTY(BlueprintReadOnly, Category = "Speech")
	FString Language;

	UPROPERTY(BlueprintReadOnly, Category = "Speech")
	ESpeechEngine Engine = ESpeechEngine::WindowsOneCore;
};

// Rendered speech: raw 16-bit PCM. C++ only.
struct FSpeechClip
{
	TArray<uint8> PcmData;
	int32 SampleRate = 0;
	int32 NumChannels = 0;
	float Duration = 0.f; // seconds, exact
};

// Set to true to cancel pending jobs (thread safe).
using FCancelFlag = TSharedPtr<std::atomic<bool>, ESPMode::ThreadSafe>;
