// AudioSettingsSubsystem.cpp
#include "AudioSettingsSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

void UAudioSettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Defaults: everything full volume
	for (uint8 i = 0; i <= static_cast<uint8>(EAudioChannel::Dialogue); ++i)
	{
		Volumes.Add(static_cast<EAudioChannel>(i), 1.0f);
	}

	if (UGameplayStatics::DoesSaveGameExist(SlotName(), 0))
	{
		if (const UAudioSaveGame* Save = Cast<UAudioSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName(), 0)))
		{
			for (const TPair<EAudioChannel, float>& Pair : Save->Volumes)
			{
				Volumes.Add(Pair.Key, FMath::Clamp(Pair.Value, 0.f, 1.f));
			}
		}
	}

	// Re-apply the mix whenever a game world starts (level loads, PIE start, etc.)
	WorldInitHandle = FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UAudioSettingsSubsystem::OnWorldInit);
}

void UAudioSettingsSubsystem::Deinitialize()
{
	FWorldDelegates::OnPostWorldInitialization.Remove(WorldInitHandle);
	Super::Deinitialize();
}

float UAudioSettingsSubsystem::GetVolume(EAudioChannel Channel) const
{
	const float* Found = Volumes.Find(Channel);
	return Found ? *Found : 1.0f;
}

void UAudioSettingsSubsystem::SetVolume(EAudioChannel Channel, float NewVolume)
{
	const float Snapped = FMath::RoundToInt(FMath::Clamp(NewVolume, 0.f, 1.f) * NumSteps) / static_cast<float>(NumSteps);
	Volumes.Add(Channel, Snapped);

	if (const UGameInstance* GI = GetGameInstance())
	{
		ApplyChannel(GI->GetWorld(), Channel);
	}
}

void UAudioSettingsSubsystem::SaveSettings() const
{
	if (UAudioSaveGame* Save = Cast<UAudioSaveGame>(UGameplayStatics::CreateSaveGameObject(UAudioSaveGame::StaticClass())))
	{
		Save->Volumes = Volumes;
		UGameplayStatics::SaveGameToSlot(Save, SlotName(), 0);
	}
}

void UAudioSettingsSubsystem::OnWorldInit(UWorld* World, const UWorld::InitializationValues IVS)
{
	if (!World || !World->IsGameWorld())
	{
		return;
	}

	for (const TPair<EAudioChannel, float>& Pair : Volumes)
	{
		ApplyChannel(World, Pair.Key);
	}
}

void UAudioSettingsSubsystem::ApplyChannel(UWorld* World, EAudioChannel Channel) const
{
	if (!World || !World->IsGameWorld())
	{
		return;
	}

	const UAudioOptionsSettings* Settings = GetDefault<UAudioOptionsSettings>();
	USoundMix* Mix = Settings->VolumeMix.LoadSynchronous();
	const TSoftObjectPtr<USoundClass>* ClassPtr = Settings->SoundClasses.Find(Channel);
	USoundClass* SoundClass = ClassPtr ? ClassPtr->LoadSynchronous() : nullptr;

	if (!Mix)
	{
		UE_LOG(LogTemp, Warning, TEXT("AudioOptions: no Volume Mix set in Project Settings > Game > Audio Options."));
		return;
	}
	if (!SoundClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("AudioOptions: no Sound Class assigned for channel %d in Project Settings > Game > Audio Options."), static_cast<int32>(Channel));
		return;
	}

	EnsureMixActive(World, Mix);
	UGameplayStatics::SetSoundMixClassOverride(World, Mix, SoundClass, GetVolume(Channel), 1.0f, 0.0f, true);
}

void UAudioSettingsSubsystem::EnsureMixActive(UWorld* World, USoundMix* Mix) const
{
	// The override only works on a mix that has been pushed in the current world, so push it once per world.
	if (MixPushedWorld.Get() != World)
	{
		UGameplayStatics::PushSoundMixModifier(World, Mix);
		MixPushedWorld = World;
	}
}