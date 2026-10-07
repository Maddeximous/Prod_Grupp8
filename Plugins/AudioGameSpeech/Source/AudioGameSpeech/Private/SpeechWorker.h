#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "Containers/Queue.h"
#include "SpeechTypes.h"
#include <atomic>

class ISpeechBackend;
class FRunnableThread;
class FEvent;

// One request to render speech.
struct FSpeechJob
{
	FString Text;
	FString VoiceId;
	float Rate = 1.f;

	// Skip the job if this is true when its turn comes.
	FCancelFlag Cancelled;

	// Only warm up the engine for VoiceId/Rate (no audio, OnDone not called).
	bool bPrepareOnly = false;

	// Called on the WORKER thread. Hop to the game thread yourself.
	TFunction<void(bool bOk, FSpeechClip&& Clip)> OnDone;
};

// Background thread that renders speech so the game never stalls.
class FSpeechWorker : public FRunnable
{
public:
	explicit FSpeechWorker(TSharedRef<ISpeechBackend> InBackend);
	virtual ~FSpeechWorker() override;

	void Enqueue(FSpeechJob&& Job);

	// FRunnable
	virtual uint32 Run() override;
	virtual void Stop() override;

private:
	// The active engine. The subsystem makes a new worker when the engine changes.
	TSharedRef<ISpeechBackend> Backend;

	TQueue<FSpeechJob, EQueueMode::Mpsc> Jobs;
	FEvent* WakeEvent = nullptr;
	FRunnableThread* Thread = nullptr;
	std::atomic<bool> bRunning{ true };
};
