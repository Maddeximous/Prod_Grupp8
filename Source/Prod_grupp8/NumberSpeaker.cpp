#include "NumberSpeaker.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Sound/SoundBase.h"

ANumberSpeaker::ANumberSpeaker()
{
    PrimaryActorTick.bCanEverTick = false;

    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    AudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComp"));
    AudioComp->SetupAttachment(Root);
    AudioComp->bAutoActivate = false;
}

void ANumberSpeaker::BeginPlay()
{
    Super::BeginPlay();
    AudioComp->OnAudioFinished.AddDynamic(this, &ANumberSpeaker::HandleClipFinished);
}

void ANumberSpeaker::AppendWholeNumber(int32 N)
{
    auto AddSmall = [this](int32 Idx)
    {
        if (ZeroToTwenty.IsValidIndex(Idx)) Queue.Add(ZeroToTwenty[Idx]);
    };

    // Over 999: read digit by digit
    if (N < 0 || N > 999)
    {
        for (TCHAR C : FString::FromInt(N)) AddSmall(C - '0');
        return;
    }

    const int32 Hundreds = N / 100;
    const int32 Rest     = N % 100;

    if (Hundreds > 0)
    {
        if (Hundreds > 1) AddSmall(Hundreds);   // 200+ -> "two" "hundred"
        Queue.Add(Hundred);
    }

    if (Rest == 0)
    {
        if (Hundreds == 0) AddSmall(0);         // plain zero
        return;
    }

    if (Rest <= 20)
    {
        AddSmall(Rest);
    }
    else
    {
        const int32 TensIdx = (Rest / 10) - 2; // 20 -> 0, 30 -> 1 ...
        if (Tens.IsValidIndex(TensIdx)) Queue.Add(Tens[TensIdx]);
        if (Rest % 10 != 0) AddSmall(Rest % 10);
    }
}

void ANumberSpeaker::BuildQueue(float Value, int32 DecimalPlaces, bool bTrim)
{
    Queue.Reset();

    DecimalPlaces = FMath::Clamp(DecimalPlaces, 0, 8);
    const FString S = FString::Printf(TEXT("%.*f"), DecimalPlaces, FMath::Abs(Value));

    FString Whole, Frac;
    if (!S.Split(TEXT("."), &Whole, &Frac)) Whole = S;

    if (bTrim)
    {
        while (Frac.EndsWith(TEXT("0"))) Frac.LeftChopInline(1);
    }

    bool bFracAllZero = true;
    for (TCHAR C : Frac) { if (C != '0') { bFracAllZero = false; break; } }

    const int32 WholeInt = FCString::Atoi(*Whole);

    // Only say "minus" if the spoken value isn't zero
    if (Value < 0.f && !(WholeInt == 0 && bFracAllZero) && Minus)
    {
        Queue.Add(Minus);
    }

    AppendWholeNumber(WholeInt);

    if (!Frac.IsEmpty())
    {
        Queue.Add(DecimalPoint);
        for (TCHAR C : Frac)
        {
            const int32 D = C - '0';
            if (ZeroToTwenty.IsValidIndex(D)) Queue.Add(ZeroToTwenty[D]);
        }
    }
}

float ANumberSpeaker::SpeakNumber(float Value, int32 DecimalPlaces, bool bTrimTrailingZeros)
{
    StopSpeaking();
    BuildQueue(Value, DecimalPlaces, bTrimTrailingZeros);

    float TotalDuration = 0.f;
    for (const TObjectPtr<USoundBase>& Clip : Queue)
    {
        if (!Clip) continue;
        const float D = Clip->GetDuration();
        if (D < INDEFINITELY_LOOPING_DURATION) // ignore looping sounds
        {
            TotalDuration += D;
        }
    }

    if (Queue.Num() == 0) return 0.f;

    bSpeaking = true;
    PlayNext();
    return TotalDuration;
}

void ANumberSpeaker::PlayNext()
{
    while (Queue.Num() > 0)
    {
        USoundBase* Next = Queue[0];
        Queue.RemoveAt(0);
        if (!Next) continue;               // skip unassigned slots

        AudioComp->SetSound(Next);
        AudioComp->Play();
        return;
    }

    // Queue exhausted
    if (bSpeaking)
    {
        bSpeaking = false;
        OnSpeechFinished.Broadcast();
    }
}

void ANumberSpeaker::HandleClipFinished()
{
    if (bSpeaking) PlayNext();
}

void ANumberSpeaker::StopSpeaking()
{
    Queue.Reset();
    bSpeaking = false;
    if (AudioComp) AudioComp->Stop();
}