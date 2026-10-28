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
    
    /** Speak a number followed by the kilogram clip. */
    UFUNCTION(BlueprintCallable, Category="NumberSpeech")
    float SpeakKilo(float Value, int32 DecimalPlaces = 2, bool bTrimTrailingZeros = true);

    /** Speak a number followed by the kronor clip. */
    UFUNCTION(BlueprintCallable, Category="NumberSpeech")
    float SpeakSek(float Value, int32 DecimalPlaces = 2, bool bTrimTrailingZeros = true);
    
    /** Start a new spoken sequence. Interrupts anything currently playing. */
    UFUNCTION(BlueprintCallable, Category="NumberSpeech")
    void BeginSequence();

    /** Add any clip to the sequence. Null clips are ignored. */
    UFUNCTION(BlueprintCallable, Category="NumberSpeech")
    void QueueClip(USoundBase* Clip);

    /** Add a spoken number (and an optional clip after it) to the sequence. */
    UFUNCTION(BlueprintCallable, Category="NumberSpeech")
    void QueueNumber(float Value, int32 DecimalPlaces = 0, bool bTrimTrailingZeros = true, USoundBase* Suffix = nullptr);

    /** Start playing the queued sequence. Returns total duration. */
    UFUNCTION(BlueprintCallable, Category="NumberSpeech")
    float PlaySequence();

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
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NumberSpeech|Clips")
    TObjectPtr<USoundBase> Kilogram;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NumberSpeech|Clips")
    TObjectPtr<USoundBase> Kronor;

protected:
    virtual void BeginPlay() override;

private:
    UFUNCTION()
    void HandleClipFinished();

    void PlayNext();
    void BuildQueue(float Value, int32 DecimalPlaces, bool bTrim);
    void AppendWholeNumber(int32 N);
    float SpeakWithSuffix(float Value, int32 DecimalPlaces, bool bTrim, USoundBase* Suffix);
    void AppendNumber(float Value, int32 DecimalPlaces, bool bTrim);

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UAudioComponent> AudioComp;

    UPROPERTY(Transient)
    TArray<TObjectPtr<USoundBase>> Queue;

    bool bSpeaking = false;
};