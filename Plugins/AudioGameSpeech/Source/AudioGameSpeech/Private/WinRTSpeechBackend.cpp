#include "WinRTSpeechBackend.h"

#if WITH_WINRT_SPEECH

#include "SpeechLog.h"
#include "SpeechWav.h"

#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/AllowWindowsPlatformAtomics.h"
THIRD_PARTY_INCLUDES_START
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Media.SpeechSynthesis.h>
#include <winrt/Windows.Storage.Streams.h>
THIRD_PARTY_INCLUDES_END
#include "Windows/HideWindowsPlatformAtomics.h"
#include "Windows/HideWindowsPlatformTypes.h"

using namespace winrt::Windows::Media::SpeechSynthesis;
using winrt::Windows::Storage::Streams::DataReader;

struct FWinRTSpeechBackend::FImpl
{
	SpeechSynthesizer Synth{ nullptr };
	FString CurrentVoiceId;
	bool bApartmentInitialized = false;
};

FWinRTSpeechBackend::FWinRTSpeechBackend() : Impl(MakeUnique<FImpl>()) {}
FWinRTSpeechBackend::~FWinRTSpeechBackend() = default;

void FWinRTSpeechBackend::InitThread()
{
	// Blocking WinRT calls need a multi-threaded apartment
	try
	{
		winrt::init_apartment(winrt::apartment_type::multi_threaded);
		Impl->bApartmentInitialized = true;
	}
	catch (...) {} // already initialized: fine

	try
	{
		Impl->Synth = SpeechSynthesizer();
	}
	catch (winrt::hresult_error const& E)
	{
		UE_LOG(LogAudioGameSpeech, Error, TEXT("Could not create speech synthesizer: %s"), E.message().c_str());
	}
}

void FWinRTSpeechBackend::ShutdownThread()
{
	Impl->Synth = nullptr;
	if (Impl->bApartmentInitialized)
	{
		winrt::uninit_apartment();
		Impl->bApartmentInitialized = false;
	}
}

TArray<FSpeechVoice> FWinRTSpeechBackend::GetVoices()
{
	TArray<FSpeechVoice> Result;
	try
	{
		for (VoiceInformation const& Info : SpeechSynthesizer::AllVoices())
		{
			FSpeechVoice Voice;
			Voice.Id          = Info.Id().c_str();
			Voice.DisplayName = Info.DisplayName().c_str();
			Voice.Language    = Info.Language().c_str();
			Voice.Engine      = ESpeechEngine::WindowsOneCore;
			Result.Add(MoveTemp(Voice));
		}
	}
	catch (winrt::hresult_error const& E)
	{
		UE_LOG(LogAudioGameSpeech, Error, TEXT("Could not list voices: %s"), E.message().c_str());
	}
	return Result;
}

bool FWinRTSpeechBackend::Synthesize(const FString& Text, const FString& VoiceId, float Rate, FSpeechClip& OutClip)
{
	if (!Impl->Synth)
	{
		return false;
	}

	try
	{
		// Switch voice only when it changed
		if (!VoiceId.IsEmpty() && VoiceId != Impl->CurrentVoiceId)
		{
			for (VoiceInformation const& Info : SpeechSynthesizer::AllVoices())
			{
				if (VoiceId == Info.Id().c_str())
				{
					Impl->Synth.Voice(Info);
					Impl->CurrentVoiceId = VoiceId;
					break;
				}
			}
		}

		// Speed: 1.0 = normal
		Impl->Synth.Options().SpeakingRate(FMath::Clamp((double)Rate, 0.5, 6.0));

		// Text -> WAV in memory
		SpeechSynthesisStream Stream = Impl->Synth.SynthesizeTextToStreamAsync(winrt::hstring(*Text)).get();

		const uint32 Size = (uint32)Stream.Size();
		if (Size == 0)
		{
			return false;
		}

		TArray<uint8> Wav;
		Wav.SetNumUninitialized((int32)Size);

		DataReader Reader(Stream.GetInputStreamAt(0));
		Reader.LoadAsync(Size).get();
		Reader.ReadBytes(winrt::array_view<uint8_t>(Wav.GetData(), Wav.GetData() + Size));

		return ParseWav(Wav, OutClip);
	}
	catch (winrt::hresult_error const& E)
	{
		UE_LOG(LogAudioGameSpeech, Warning, TEXT("Speech failed: %s"), E.message().c_str());
	}
	catch (...)
	{
		UE_LOG(LogAudioGameSpeech, Warning, TEXT("Speech failed (unknown error)"));
	}
	return false;
}

#endif // WITH_WINRT_SPEECH
