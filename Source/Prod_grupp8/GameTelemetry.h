#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameTelemetry.generated.h"

UCLASS()
class PROD_GRUPP8_API UGameTelemetry : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Kalla på denna varje gång spelaren klickar på 'A'
	void RecordAButtonPress();

	// Kalla på dessa när tidsförloppet för att reela in startar och slutar
	void StartReeling();
	void EndReeling(bool bCaughtFish);

	// Exportera till JSON-fil
	void SaveTelemetryToJson();

private:
	double SessionStartTime = 0.0;
	bool bHasCaughtFirstFish = false;
	float TimeToFirstCatch = 0.0f;

	int32 AButtonPressCount = 0;

	double ReelStartTime = 0.0;
	TArray<float> ReelInDurations;

	float GetAverageReelTime() const;
};