// Fill out your copyright notice in the Description page of Project Settings.

#include "OnboardingSubsystem.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "SpeechSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogOnboarding, Log, All);

namespace
{
	const FName BoatMovedGate(TEXT("BoatMoved"));
}

void UOnboardingSubsystem::StartOnboarding(UDataTable* Script)
{
	if (!Script)
	{
		UE_LOG(LogOnboarding, Warning, TEXT("StartOnboarding: no DataTable set."));
		return;
	}

	Steps.Reset();
	TArray<FOnboardingStep*> Rows;
	Script->GetAllRows<FOnboardingStep>(TEXT("Onboarding"), Rows);
	TArray<FText> Lines;
	for (const FOnboardingStep* Row : Rows)
	{
		Steps.Add(*Row);
		if (Row->Type == EOnboardingStepType::Speak)
		{
			Lines.Add(Row->Text);
		}
	}

	// Render all lines now so they play without delay
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (USpeechSubsystem* Speech = GI->GetSubsystem<USpeechSubsystem>())
		{
			Speech->Precache(Lines);
		}
	}

	OpenGates.Reset();
	bRunning = true;
	UE_LOG(LogOnboarding, Log, TEXT("Onboarding started (%d steps)"), Steps.Num());
	StartStep(0);
}

void UOnboardingSubsystem::Skip()
{
	if (!bRunning)
	{
		return;
	}
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (USpeechSubsystem* Speech = GI->GetSubsystem<USpeechSubsystem>())
		{
			Speech->StopSpeaking();
		}
	}
	UE_LOG(LogOnboarding, Log, TEXT("Onboarding skipped"));
	Finish();
}

void UOnboardingSubsystem::ReportEvent(FName Gate)
{
	if (!bRunning || OpenGates.Contains(Gate))
	{
		return;
	}
	OpenGates.Add(Gate);
	UE_LOG(LogOnboarding, Log, TEXT("Gate open: %s"), *Gate.ToString());
}

void UOnboardingSubsystem::ReportOnboardingEvent(const UObject* WorldContextObject, FName Gate)
{
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	if (UOnboardingSubsystem* Onboarding = World ? World->GetSubsystem<UOnboardingSubsystem>() : nullptr)
	{
		Onboarding->ReportEvent(Gate);
	}
}

void UOnboardingSubsystem::Tick(float DeltaTime)
{
	if (!bRunning || !Steps.IsValidIndex(StepIndex))
	{
		return;
	}

	StepTime += DeltaTime;

	// Step done: wait out DelayAfter, then go on
	if (bStepDone)
	{
		DelayLeft -= DeltaTime;
		if (DelayLeft <= 0.f)
		{
			StartStep(StepIndex + 1);
		}
		return;
	}

	const FOnboardingStep& Step = Steps[StepIndex];
	switch (Step.Type)
	{
	case EOnboardingStepType::Speak:
		if (!IsSpeaking())
		{
			FinishStep();
		}
		break;

	case EOnboardingStepType::Sound:
		if (StepTime >= SoundDuration)
		{
			FinishStep();
		}
		break;

	case EOnboardingStepType::Wait:
		if (Step.Gate == BoatMovedGate && FVector::Dist2D(GetPawnLocation(), WaitStartLocation) >= BoatMoveDistance)
		{
			ReportEvent(BoatMovedGate);
		}
		if (OpenGates.Contains(Step.Gate))
		{
			FinishStep();
			break;
		}
		// Player stuck: repeat the instruction
		HintTime += DeltaTime;
		if (Step.HintAfter > 0.f && HintTime >= Step.HintAfter && !IsSpeaking())
		{
			HintTime = 0.f;
			Say(Step.Text.IsEmpty() ? LastLine : Step.Text);
		}
		break;
	}
}

void UOnboardingSubsystem::StartStep(int32 Index)
{
	if (!Steps.IsValidIndex(Index))
	{
		Finish();
		return;
	}

	StepIndex = Index;
	bStepDone = false;
	StepTime = 0.f;
	HintTime = 0.f;

	const FOnboardingStep& Step = Steps[Index];
	switch (Step.Type)
	{
	case EOnboardingStepType::Speak:
		LastLine = Step.Text;
		Say(Step.Text);
		UE_LOG(LogOnboarding, Log, TEXT("Step %d: speak \"%s\""), Index, *Step.Text.ToString());
		break;

	case EOnboardingStepType::Sound:
	{
		USoundBase* Sound = Step.Sound.LoadSynchronous();
		SoundDuration = Sound ? Sound->GetDuration() : 0.f;
		if (Sound)
		{
			UGameplayStatics::PlaySound2D(this, Sound);
		}
		else
		{
			UE_LOG(LogOnboarding, Warning, TEXT("Step %d: no sound set, skipping"), Index);
		}
		break;
	}

	case EOnboardingStepType::Wait:
		WaitStartLocation = GetPawnLocation();
		UE_LOG(LogOnboarding, Log, TEXT("Step %d: waiting for %s"), Index, *Step.Gate.ToString());
		break;
	}

	OnStepStarted.Broadcast(Index);
}

void UOnboardingSubsystem::FinishStep()
{
	bStepDone = true;
	DelayLeft = Steps[StepIndex].DelayAfter;
}

void UOnboardingSubsystem::Finish()
{
	bRunning = false;
	StepIndex = INDEX_NONE;
	UE_LOG(LogOnboarding, Log, TEXT("Onboarding finished"));
	OnOnboardingFinished.Broadcast();
}

void UOnboardingSubsystem::Say(const FText& Text)
{
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		if (USpeechSubsystem* Speech = GI->GetSubsystem<USpeechSubsystem>())
		{
			Speech->Speak(Text, false);
		}
	}
}

bool UOnboardingSubsystem::IsSpeaking() const
{
	UGameInstance* GI = GetWorld()->GetGameInstance();
	USpeechSubsystem* Speech = GI ? GI->GetSubsystem<USpeechSubsystem>() : nullptr;
	return Speech && Speech->IsSpeaking();
}

FVector UOnboardingSubsystem::GetPawnLocation() const
{
	APawn* Pawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	return Pawn ? Pawn->GetActorLocation() : FVector::ZeroVector;
}

TStatId UOnboardingSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UOnboardingSubsystem, STATGROUP_Tickables);
}

bool UOnboardingSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}
