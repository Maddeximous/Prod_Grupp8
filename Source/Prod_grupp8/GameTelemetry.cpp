#include "GameTelemetry.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformTime.h"
#include "HAL/FileManager.h"

void UGameTelemetry::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    SessionStartTime = FPlatformTime::Seconds();
}

void UGameTelemetry::Deinitialize()
{
    // Sparar automatiskt när spelet stängs eller GameInstance förstörs
    SaveTelemetryToJson();
    Super::Deinitialize();
}

void UGameTelemetry::RecordAButtonPress()
{
    AButtonPressCount++;
}

void UGameTelemetry::StartReeling()
{
    ReelStartTime = FPlatformTime::Seconds();
}

void UGameTelemetry::EndReeling(bool bCaughtFish)
{
    if (ReelStartTime <= 0.0) return;

    double Duration = FPlatformTime::Seconds() - ReelStartTime;
    ReelStartTime = 0.0;

    if (bCaughtFish)
    {
        ReelInDurations.Add(static_cast<float>(Duration));

        if (!bHasCaughtFirstFish)
        {
            bHasCaughtFirstFish = true;
            TimeToFirstCatch = static_cast<float>(FPlatformTime::Seconds() - SessionStartTime);
        }
    }
}

float UGameTelemetry::GetAverageReelTime() const
{
    if (ReelInDurations.Num() == 0) return 0.0f;

    float Total = 0.0f;
    for (float Duration : ReelInDurations)
    {
        Total += Duration;
    }
    return Total / ReelInDurations.Num();
}

void UGameTelemetry::SaveTelemetryToJson()
{
    TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();

    JsonObject->SetNumberField(TEXT("time_to_first_catch_seconds"), TimeToFirstCatch);
    JsonObject->SetNumberField(TEXT("a_button_press_count"), AButtonPressCount);
    JsonObject->SetNumberField(TEXT("average_reel_time_seconds"), GetAverageReelTime());
    JsonObject->SetNumberField(TEXT("total_fish_caught"), static_cast<double>(ReelInDurations.Num()));

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    FString Directory = FPaths::ProjectSavedDir() / TEXT("PlaytestData");
    IFileManager::Get().MakeDirectory(*Directory, true);

    FString FilePath = Directory / FString::Printf(TEXT("Telemetry_%s.json"), *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));

    FFileHelper::SaveStringToFile(OutputString, *FilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_None);
}