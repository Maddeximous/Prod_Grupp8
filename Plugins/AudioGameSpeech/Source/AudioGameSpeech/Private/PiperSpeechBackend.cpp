#include "PiperSpeechBackend.h"
#include "SpeechLog.h"
#include "SpeechWav.h"

#include "HAL/FileManager.h"
#include "HAL/PlatformTime.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace
{
	const TCHAR* VoicePrefix = TEXT("Piper:");
	constexpr double LineTimeout = 30.0; // seconds (includes model load on first line)

	// "sv_SE-nst-medium" -> "sv-SE"
	FString LanguageFromVoiceName(const FString& Name)
	{
		FString Code = Name;
		Name.Split(TEXT("-"), &Code, nullptr);
		return Code.Replace(TEXT("_"), TEXT("-"));
	}

	FString EscapeJson(const FString& In)
	{
		FString Out;
		Out.Reserve(In.Len() + 8);
		for (const TCHAR C : In)
		{
			switch (C)
			{
			case TEXT('"'):  Out += TEXT("\\\""); break;
			case TEXT('\\'): Out += TEXT("\\\\"); break;
			case TEXT('\n'): Out += TEXT("\\n"); break;
			case TEXT('\r'): Out += TEXT("\\r"); break;
			case TEXT('\t'): Out += TEXT("\\t"); break;
			default:
				if (C < 0x20)
				{
					Out += FString::Printf(TEXT("\\u%04x"), (uint32)C);
				}
				else
				{
					Out.AppendChar(C);
				}
			}
		}
		return Out;
	}
}

FString FPiperSpeechBackend::FindPiperDir()
{
	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("AudioGameSpeech"));
	if (!Plugin.IsValid())
	{
		return FString();
	}
	const FString Dir = FPaths::ConvertRelativePathToFull(Plugin->GetBaseDir() / TEXT("ThirdParty/Piper"));
	return FPaths::FileExists(Dir / TEXT("piper.exe")) ? Dir : FString();
}

FPiperSpeechBackend::FPiperSpeechBackend(const FString& InPiperDir)
	: PiperDir(InPiperDir)
{
	// Saved/ is writable (install folder may not be). Passed as the working directory,
	// so Piper only sees relative file names (its narrow-char paths break on å/ä/ö in user names).
	WorkDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("Speech/Piper"));
	IFileManager::Get().MakeDirectory(*WorkDir, true);

	// Leftovers from a crash
	TArray<FString> OldFiles;
	IFileManager::Get().FindFiles(OldFiles, *(WorkDir / TEXT("*.wav")), true, false);
	for (const FString& File : OldFiles)
	{
		IFileManager::Get().Delete(*(WorkDir / File));
	}
}

FPiperSpeechBackend::~FPiperSpeechBackend()
{
	StopProcess();
}

void FPiperSpeechBackend::ShutdownThread()
{
	StopProcess();
}

TArray<FSpeechVoice> FPiperSpeechBackend::GetVoices()
{
	TArray<FSpeechVoice> Result;

	const FString VoiceDir = PiperDir / TEXT("voices");
	TArray<FString> Models;
	IFileManager::Get().FindFiles(Models, *(VoiceDir / TEXT("*.onnx")), true, false);

	for (const FString& File : Models)
	{
		// Each model needs its config next to it
		if (!File.EndsWith(TEXT(".onnx")) || !FPaths::FileExists(VoiceDir / File + TEXT(".json")))
		{
			continue;
		}

		// "sv_SE-nst-medium" -> "Piper nst"
		const FString Name = FPaths::GetBaseFilename(File);
		TArray<FString> Parts;
		Name.ParseIntoArray(Parts, TEXT("-"));

		FSpeechVoice Voice;
		Voice.Id          = VoicePrefix + Name;
		Voice.DisplayName = TEXT("Piper ") + (Parts.Num() > 1 ? Parts[1] : Name);
		Voice.Language    = LanguageFromVoiceName(Name);
		Voice.Engine      = ESpeechEngine::Piper;
		Result.Add(MoveTemp(Voice));
	}
	return Result;
}

void FPiperSpeechBackend::Prepare(const FString& VoiceId, float Rate)
{
	EnsureProcess(VoiceId, Rate);
}

bool FPiperSpeechBackend::Synthesize(const FString& Text, const FString& VoiceId, float Rate, FSpeechClip& OutClip)
{
	if (Text.IsEmpty() || !EnsureProcess(VoiceId, Rate))
	{
		return false;
	}

	const FString FileName = FString::Printf(TEXT("line_%u.wav"), ++LineCounter);
	const FString Request = FString::Printf(TEXT("{\"text\":\"%s\",\"output_file\":\"%s\"}\n"), *EscapeJson(Text), *FileName);

	// Piper reads UTF-8. (The FString WritePipe overload would mangle å/ä/ö.)
	const FTCHARToUTF8 Utf8(*Request);
	if (!FPlatformProcess::WritePipe(StdinWrite, (const uint8*)Utf8.Get(), Utf8.Length()) || !WaitForFile(FileName))
	{
		StopProcess(); // out of sync or dead: restart on next line
		return false;
	}

	const FString Path = WorkDir / FileName;
	TArray<uint8> Wav;
	const bool bLoaded = FFileHelper::LoadFileToArray(Wav, *Path);
	IFileManager::Get().Delete(*Path);

	return bLoaded && ParseWav(Wav, OutClip);
}

// ---------- Process ----------

bool FPiperSpeechBackend::EnsureProcess(const FString& VoiceId, float Rate)
{
	const bool bRunning = Proc.IsValid() && FPlatformProcess::IsProcRunning(Proc);
	if (bRunning && VoiceId == RunningVoiceId && FMath::IsNearlyEqual(Rate, RunningRate, 0.01f))
	{
		return true;
	}

	// Voice or speed are fixed per process: restart (~1 s)
	StopProcess();
	return StartProcess(VoiceId, Rate);
}

bool FPiperSpeechBackend::StartProcess(const FString& VoiceId, float Rate)
{
	FString Name = VoiceId;
	if (!Name.RemoveFromStart(VoicePrefix))
	{
		UE_LOG(LogAudioGameSpeech, Warning, TEXT("Not a Piper voice: %s"), *VoiceId);
		return false;
	}

	const FString Model = PiperDir / TEXT("voices") / Name + TEXT(".onnx");
	if (!FPaths::FileExists(Model))
	{
		UE_LOG(LogAudioGameSpeech, Error, TEXT("Piper voice missing: %s"), *Model);
		return false;
	}

	// Piper stretches phonemes: 2x speed = half length
	const float LengthScale = 1.f / FMath::Clamp(Rate, 0.5f, 6.0f);
	const FString Exe = PiperDir / TEXT("piper.exe");
	const FString Params = FString::Printf(TEXT("--model \"%s\" --json-input --output_dir . --length_scale %.3f"), *Model, LengthScale);

	FPlatformProcess::CreatePipe(StdoutRead, StdoutWrite);       // we read, Piper writes
	FPlatformProcess::CreatePipe(StdinRead, StdinWrite, true);   // we write, Piper reads

	Proc = FPlatformProcess::CreateProc(*Exe, *Params,
		/*bLaunchDetached*/ false, /*bLaunchHidden*/ true, /*bLaunchReallyHidden*/ true,
		nullptr, 0, *WorkDir, StdoutWrite, StdinRead);

	if (!Proc.IsValid())
	{
		UE_LOG(LogAudioGameSpeech, Error, TEXT("Could not start %s"), *Exe);
		StopProcess();
		return false;
	}

	RunningVoiceId = VoiceId;
	RunningRate = Rate;
	UE_LOG(LogAudioGameSpeech, Log, TEXT("Piper started: %s, speed %.2f"), *Name, Rate);
	return true;
}

void FPiperSpeechBackend::StopProcess()
{
	if (Proc.IsValid())
	{
		// Closing stdin makes Piper exit by itself
		FPlatformProcess::ClosePipe(nullptr, StdinWrite);
		StdinWrite = nullptr;

		for (int32 i = 0; i < 50 && FPlatformProcess::IsProcRunning(Proc); ++i)
		{
			FPlatformProcess::Sleep(0.01f);
		}
		if (FPlatformProcess::IsProcRunning(Proc))
		{
			FPlatformProcess::TerminateProc(Proc, true);
		}
		FPlatformProcess::CloseProc(Proc);
	}

	FPlatformProcess::ClosePipe(StdinRead, StdinWrite);
	FPlatformProcess::ClosePipe(StdoutRead, StdoutWrite);
	StdinRead = StdinWrite = StdoutRead = StdoutWrite = nullptr;

	PendingOutput.Reset();
	RunningVoiceId.Reset();
}

bool FPiperSpeechBackend::WaitForFile(const FString& FileName)
{
	// Piper prints the WAV path when it's written. Everything else is its log (stderr shares the pipe).
	const double EndTime = FPlatformTime::Seconds() + LineTimeout;
	while (FPlatformTime::Seconds() < EndTime)
	{
		PendingOutput += FPlatformProcess::ReadPipe(StdoutRead);

		int32 NewLine;
		while (PendingOutput.FindChar(TEXT('\n'), NewLine))
		{
			const FString Line = PendingOutput.Left(NewLine).TrimStartAndEnd();
			PendingOutput.RightChopInline(NewLine + 1);

			if (FPaths::GetCleanFilename(Line) == FileName)
			{
				return true;
			}
			if (Line.Contains(TEXT("[error]")))
			{
				UE_LOG(LogAudioGameSpeech, Warning, TEXT("Piper: %s"), *Line);
			}
			else if (!Line.IsEmpty())
			{
				UE_LOG(LogAudioGameSpeech, Verbose, TEXT("Piper: %s"), *Line);
			}
		}

		if (!FPlatformProcess::IsProcRunning(Proc))
		{
			int32 Code = 0;
			FPlatformProcess::GetProcReturnCode(Proc, &Code);
			UE_LOG(LogAudioGameSpeech, Warning, TEXT("Piper stopped (exit code %d). %s"), Code, *PendingOutput);
			return false;
		}

		FPlatformProcess::Sleep(0.005f);
	}

	UE_LOG(LogAudioGameSpeech, Warning, TEXT("Piper timed out on a line, restarting it."));
	return false;
}
