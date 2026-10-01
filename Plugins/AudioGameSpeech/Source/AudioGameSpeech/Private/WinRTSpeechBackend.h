#pragma once

#include "SpeechBackend.h"

#if WITH_WINRT_SPEECH

// Windows OneCore voices (Microsoft Bengt etc.) via Windows.Media.SpeechSynthesis.
class FWinRTSpeechBackend final : public ISpeechBackend
{
public:
	FWinRTSpeechBackend();
	virtual ~FWinRTSpeechBackend() override;

	virtual void InitThread() override;
	virtual void ShutdownThread() override;
	virtual TArray<FSpeechVoice> GetVoices() override;
	virtual bool Synthesize(const FString& Text, const FString& VoiceId, float Rate, FSpeechClip& OutClip) override;

private:
	struct FImpl; // hides WinRT types from other files
	TUniquePtr<FImpl> Impl;
};

#endif
