#include "SpeechWorker.h"
#include "SpeechBackend.h"
#include "HAL/RunnableThread.h"
#include "HAL/Event.h"
#include "HAL/PlatformProcess.h"

FSpeechWorker::FSpeechWorker(TSharedRef<ISpeechBackend> InBackend)
	: Backend(InBackend)
{
	WakeEvent = FPlatformProcess::GetSynchEventFromPool(false);
	Thread = FRunnableThread::Create(this, TEXT("AudioGameSpeechWorker"), 0, TPri_Normal);
}

FSpeechWorker::~FSpeechWorker()
{
	if (Thread)
	{
		Thread->Kill(true); // calls Stop(), waits for Run() to exit
		delete Thread;
		Thread = nullptr;
	}
	FPlatformProcess::ReturnSynchEventToPool(WakeEvent);
	WakeEvent = nullptr;
}

void FSpeechWorker::Enqueue(FSpeechJob&& Job)
{
	Jobs.Enqueue(MoveTemp(Job));
	WakeEvent->Trigger();
}

uint32 FSpeechWorker::Run()
{
	Backend->InitThread();

	while (bRunning)
	{
		FSpeechJob Job;
		while (bRunning && Jobs.Dequeue(Job))
		{
			// Interrupted or settings changed: skip
			if (Job.Cancelled.IsValid() && Job.Cancelled->load())
			{
				continue;
			}

			if (Job.bPrepareOnly)
			{
				Backend->Prepare(Job.VoiceId, Job.Rate);
				continue;
			}

			FSpeechClip Clip;
			const bool bOk = Backend->Synthesize(Job.Text, Job.VoiceId, Job.Rate, Clip);

			if (Job.OnDone)
			{
				Job.OnDone(bOk, MoveTemp(Clip));
			}
		}

		WakeEvent->Wait(100); // wakes on Enqueue, or every 100 ms
	}

	Backend->ShutdownThread();
	return 0;
}

void FSpeechWorker::Stop()
{
	bRunning = false;
	WakeEvent->Trigger();
}
