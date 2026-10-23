// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "OnboardingTypes.generated.h"

class USoundBase;

UENUM(BlueprintType)
enum class EOnboardingStepType : uint8
{
	Speak,	// Text line, not voiced yet: continues right away
	Sound,	// Play Sound, continue when finished
	Wait	// Stop until Gate is open (game calls ReportOnboardingEvent)
};

// One row in DT_Onboarding (Content/Onboarding/Onboarding.csv)
USTRUCT(BlueprintType)
struct FOnboardingStep : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Onboarding")
	EOnboardingStepType Type = EOnboardingStepType::Speak;

	// Speak: the line (not voiced yet, recorded lines come later). Wait: hint text
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Onboarding", meta = (MultiLine = true))
	FText Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Onboarding")
	TSoftObjectPtr<USoundBase> Sound;

	// Wait: event name, e.g. SonarPinged. "BoatMoved" is measured by the onboarding itself
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Onboarding")
	FName Gate;

	// Pause in seconds after the step is done
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Onboarding")
	float DelayAfter = 0.f;

	// Wait: hint interval in seconds (not used right now, kept for the DataTable)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Onboarding")
	float HintAfter = 0.f;
};
