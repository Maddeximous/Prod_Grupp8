// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OnboardingTypes.h"
#include "OnboardingSubsystem.generated.h"

class UDataTable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOnboardingStepStarted, int32, StepIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOnboardingFinished);

/**
 * Runs the tutorial from DT_Onboarding, one step at a time.
 * Wait steps stop until the game reports the event:
 *   UOnboardingSubsystem::ReportOnboardingEvent(this, "SonarPinged");
 * Events are remembered, so doing something early never gets the tutorial stuck.
 * Reporting when no tutorial runs does nothing.
 */
UCLASS()
class PROD_GRUPP8_API UOnboardingSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Onboarding")
	void StartOnboarding(UDataTable* Script);

	UFUNCTION(BlueprintCallable, Category = "Onboarding")
	void Skip();

	UFUNCTION(BlueprintCallable, Category = "Onboarding")
	void ReportEvent(FName Gate);

	// Call from anywhere: C++ or Blueprint
	UFUNCTION(BlueprintCallable, Category = "Onboarding", meta = (WorldContext = "WorldContextObject"))
	static void ReportOnboardingEvent(const UObject* WorldContextObject, FName Gate);

	UFUNCTION(BlueprintPure, Category = "Onboarding")
	bool IsGateOpen(FName Gate) const { return OpenGates.Contains(Gate); }

	UFUNCTION(BlueprintPure, Category = "Onboarding")
	bool IsRunning() const { return bRunning; }

	UPROPERTY(BlueprintAssignable, Category = "Onboarding")
	FOnOnboardingStepStarted OnStepStarted;

	UPROPERTY(BlueprintAssignable, Category = "Onboarding")
	FOnOnboardingFinished OnOnboardingFinished;

	// Distance in cm the boat must move for the "BoatMoved" gate
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Onboarding")
	float BoatMoveDistance = 500.f;

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	void StartStep(int32 Index);
	void FinishStep();
	void Finish();
	void Say(const FText& Text);
	bool IsSpeaking() const;
	FVector GetPawnLocation() const;

	TArray<FOnboardingStep> Steps;
	TSet<FName> OpenGates;
	int32 StepIndex = INDEX_NONE;
	bool bRunning = false;
	bool bStepDone = false;
	float StepTime = 0.f;		// time in current step
	float DelayLeft = 0.f;		// pause after a finished step
	float HintTime = 0.f;
	float SoundDuration = 0.f;
	FVector WaitStartLocation = FVector::ZeroVector;
	FText LastLine;				// repeated as hint if a Wait step has no text
};
