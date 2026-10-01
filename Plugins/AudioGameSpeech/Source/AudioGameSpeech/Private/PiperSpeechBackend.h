#pragma once

#include "SpeechBackend.h"
#include "HAL/PlatformProcess.h"

// Piper: offline Swedish voice shipped with the game (default engine).
// Runs piper.exe as a hidden child process: JSON lines in on stdin, WAV files out.
// A separate process keeps Piper's GPL / ONNX Runtime code out of the game binary.
// The process only runs while Piper is the active engine (started/stopped on the worker thread).
class FPiperSpeechBackend final : public ISpeechBackend
{
public:
	// Folder with piper.exe, espeak-ng-data/ and voices/. Empty if Setup-Piper.ps1 hasn't been run.
	static FString FindPiperDir();

	explicit FPiperSpeechBackend(const FString& InPiperDir);
	virtual ~FPiperSpeechBackend() override;

	virtual void ShutdownThread() override;
	virtual TArray<FSpeechVoice> GetVoices() override;
	virtual void Prepare(const FString& VoiceId, float Rate) override;
	virtual bool Synthesize(const FString& Text, const FString& VoiceId, float Rate, FSpeechClip& OutClip) override;

private:
	bool EnsureProcess(const FString& VoiceId, float Rate);
	bool StartProcess(const FString& VoiceId, float Rate);
	void StopProcess();
	bool WaitForFile(const FString& FileName);

	FString PiperDir;
	FString WorkDir; // where Piper writes WAVs (must be writable)

	FProcHandle Proc;
	void* StdinRead = nullptr;
	void* StdinWrite = nullptr;
	void* StdoutRead = nullptr;
	void* StdoutWrite = nullptr;
	FString PendingOutput;

	// Voice and speed the running process was started with
	FString RunningVoiceId;
	float RunningRate = 0.f;
	uint32 LineCounter = 0;
};
