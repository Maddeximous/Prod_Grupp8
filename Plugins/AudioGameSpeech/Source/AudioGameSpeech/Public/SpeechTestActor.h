#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpeechTestActor.generated.h"

/**
 * Test only. Drop in a level and press Play: lists voices and speaks TestLine.
 * Shows status on screen and in the log (LogAudioGameSpeech). Safe to delete later.
 */
UCLASS(Blueprintable)
class AUDIOGAMESPEECH_API ASpeechTestActor : public AActor
{
	GENERATED_BODY()

public:
	ASpeechTestActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speech Test")
	FText TestLine;

	// Queued after TestLine (Interrupt = false), to test the queue.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speech Test")
	FText QueuedLine;

	// 1.0 = normal. Range 0.5 to 6.0.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speech Test", meta = (ClampMin = "0.5", ClampMax = "6.0"))
	float SpeakingRate = 1.f;

	// Speak TestLine and QueuedLine again.
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Speech Test")
	void RunTest();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleSpeechStarted(float DurationSeconds);

	UFUNCTION()
	void HandleSpeechFinished();

	class USpeechSubsystem* GetSpeech() const;
	void Report(const FString& Message, const FColor& Color = FColor::Cyan) const;
};
