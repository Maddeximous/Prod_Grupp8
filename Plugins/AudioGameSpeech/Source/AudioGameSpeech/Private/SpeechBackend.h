#pragma once

#include "CoreMinimal.h"
#include "SpeechTypes.h"

// Interface every speech engine implements.
// Now: Piper (built in, default) and Windows OneCore (Bengt). Later: SAPI5 third-party voices.
// Only the active engine has a worker thread; switching engine shuts the old one down.
class ISpeechBackend
{
public:
	virtual ~ISpeechBackend() = default;

	// Worker thread start/stop (COM setup, child processes etc.)
	virtual void InitThread() {}
	virtual void ShutdownThread() {}

	// List installed voices. Game thread, must be cheap.
	virtual TArray<FSpeechVoice> GetVoices() = 0;

	// Get ready for this voice/speed so the first line is fast. Worker thread only.
	virtual void Prepare(const FString& VoiceId, float Rate) {}

	// Text -> PCM. Blocking. Worker thread only.
	virtual bool Synthesize(const FString& Text, const FString& VoiceId, float Rate, FSpeechClip& OutClip) = 0;
};
