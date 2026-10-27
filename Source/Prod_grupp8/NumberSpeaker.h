#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NumberSpeaker.generated.h"

class UAudioComponent;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNumberSpeechFinished);

UCLASS()
class PROD_GRUPP8_API ANumberSpeaker : public AActor
{
    GENERATED_BODY()

public:
    ANumberSpeaker();

    /** Speak a number. Interrupts anything currently being spoken. */
    UFUNCTION(BlueprintCallable, Category="NumberSpeech")
    float SpeakNumber(float Value, int32 DecimalPlaces = 2, bool bTrimTrailingZeros = true);

    UFUNCTION(BlueprintCallable, Category="NumberSpeech")
    void StopSpeaking();

    UFUNCTION(BlueprintPure, Category="NumberSpeech")
    bool IsSpeaking() const { return bSpeaking; }

    /** Fires when the last clip of a number has finished. */
    UPROPERTY(BlueprintAssignable, Category="NumberSpeech")
    FOnNumberSpeechFinished OnSpeechFinished;

    // ---- Clips, assigned in the Details panel ----

    /** Index = value. Needs 21 entries: 0, 1, 2 ... 20. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NumberSpeech|Clips")
    TArray<TObjectPtr<USoundBase>> ZeroToTwenty;

    /** Index 0 = 20, 1 = 30, 2 = 40 ... 7 = 90. Needs 8 entries. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NumberSpeech|Clips")
    TArray<TObjectPtr<USoundBase>> Tens;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NumberSpeech|Clips")
    TObjectPtr<USoundBase> Hundred;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NumberSpeech|Clips")
    TObjectPtr<USoundBase> DecimalPoint;

    /** Optional. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NumberSpeech|Clips")
    TObjectPtr<USoundBase> Minus;

protected:
    virtual void BeginPlay() override;

private:
    UFUNCTION()
    void HandleClipFinished();

    void PlayNext();
    void BuildQueue(float Value, int32 DecimalPlaces, bool bTrim);
    void AppendWholeNumber(int32 N);

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UAudioComponent> AudioComp;

    UPROPERTY(Transient)
    TArray<TObjectPtr<USoundBase>> Queue;

    bool bSpeaking = false;
};