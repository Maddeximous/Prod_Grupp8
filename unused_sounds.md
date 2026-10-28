# Oanvända ljud-assets

Genererad 2026-10-28. Skanning av alla 227 `.uasset` under `Content/Audio`.

## Sammanfattning

- Totalt: **227** assets, varav **104** troligen oanvända.
- Oanvända ljud (SoundWave/SoundCue): **98**
- Oanvända mix-/attenuation-assets (SoundClass/Submix/Mix/Attenuation): **6**
- Typer: SoundWave 196, SoundCue 17, SoundClass 6, SoundSubmix 4, SoundAttenuation 3, SoundMix 1. Inga MetaSounds hittades.

## Metod

- **Typ** avgjord från klassnamnet i assetens namntabell.
- **C++/Config:** sökning efter paketsökvägen (`/Game/Audio/...`) i `Source/` (.h/.cpp/.cs) och `Config/` (.ini). `Plugins/` finns inte i projektet. Träffar på bara namnet (t.ex. fisknamn som `Aborre` i enum:en i `FishTypes.h`) räknas inte som användning, men visas inom parentes.
- **Andra assets:** alla `.uasset`/`.umap` under `Content` lästes som binär text (ASCII/UTF-8 och UTF-16, så att å/ä/ö kommer med) och söktes efter den fullständiga paketsökvägen. Ljudets egen fil räknas inte. Med full sökväg skiljs t.ex. `Audio/SeaGull01mono` och `Audio/RawAudio/SeaGull01mono` åt.
- **Transitivt:** ett ljud som bara refereras av andra *oanvända* ljud (t.ex. en SoundWave som bara finns i en oanvänd SoundCue) räknas som oanvänt.
- "(extern actor)" = World Partition-actor som är placerad i den nivån.

## Förbehåll

- En referens från en Blueprint eller nivå som *själv* är oanvänd (t.ex. testnivåer som `Lvl_Kai`, `Level_ShopTest`) räknas ändå som användning.
- Ljud som laddas via strängar som byggs ihop vid körning, eller via Asset Manager-regler, syns inte i sökningen.
- `FiskInfo/Regnbågslax` och `FiskInfo/Röding` finns två gånger med olika Unicode-kodning av å/ö. Båda varianterna får samma resultat i tabellen och går inte att skilja åt med den här metoden.
- SoundWaves refererar till sin SoundClass, så en SoundClass räknas som använd om något använt ljud pekar på den.
- Kontrollera alltid med *Reference Viewer* i Unreal innan något tas bort.

## Tabell

| Ljud (under Content/Audio) | Typ | Hittas i C++/Config | Hittas i andra assets | Troligen oanvänt |
|---|---|---|---|---|
| `SeaGull01mono.uasset` | SoundWave | nej | nej | ja |
| `SeagullStereo.uasset` | SoundWave | nej | nej | ja |
| `23okt/FiskebodBuyRod.uasset` | SoundWave | nej | nej | ja |
| `23okt/FiskebodBuyRodShiort.uasset` | SoundWave | nej | nej | ja |
| `23okt/FiskebodSellFishShort01.uasset` | SoundWave | nej | nej | ja |
| `23okt/HavskattenFiskebodBuyFish.uasset` | SoundWave | nej | nej | ja |
| `23okt/HavskattenFiskebodBuyRod01.uasset` | SoundWave | nej | nej | ja |
| `23okt/HavskattenFiskebodBuyRodShort01.uasset` | SoundWave | nej | nej | ja |
| `23okt/HavskattenFiskebodClose.uasset` | SoundWave | nej | BP_FirstPersonCharacter | nej |
| `23okt/HavskattenFiskebodFramme_Tagn_1.uasset` | SoundWave | nej | nej | ja |
| `23okt/HavskattenFiskebodFramme_Tagn_2.uasset` | SoundWave | nej | nej | ja |
| `23okt/HavskattenFiskebodLeaveBod.uasset` | SoundWave | nej | BP_FirstPersonCharacter | nej |
| `23okt/HavskattenFiskebodSellFish.uasset` | SoundWave | nej | nej | ja |
| `23okt/HavskattenFiskebodSellFishShort.uasset` | SoundWave | nej | nej | ja |
| `23okt/HavskattenFiskebodSellFishShort02.uasset` | SoundWave | nej | nej | ja |
| `23okt/HavskattenFiskebodSellFishShort03.uasset` | SoundWave | nej | nej | ja |
| `23okt/HavskattenIntroNavigation.uasset` | SoundWave | nej | nej | ja |
| `23okt/HavskattenQuit.uasset` | SoundWave | nej | nej | ja |
| `23okt/HavskattenSettings.uasset` | SoundWave | nej | nej | ja |
| `23okt/HavskattenSettings1.uasset` | SoundWave | nej | nej | ja |
| `23okt/HavskattenSettingsSound.uasset` | SoundWave | nej | nej | ja |
| `23okt/HavskattenSettingsSoundAmbience.uasset` | SoundWave | nej | nej | ja |
| `23okt/HavskattenSettingsSoundDialogOnboarding.uasset` | SoundWave | nej | WBP_AudioOptions | nej |
| `23okt/HavskattenSettingsSoundMaster.uasset` | SoundWave | nej | nej | ja |
| `23okt/HavskattenSettingsSoundMusic.uasset` | SoundWave | nej | nej | ja |
| `23okt/HavskattenSettingsSoundSFX.uasset` | SoundWave | nej | nej | ja |
| `23okt/HavskattenSiaMeow2.uasset` | SoundWave | nej | SC_Cat | nej |
| `23okt/HavskattenSiaMeow3.uasset` | SoundWave | nej | SC_Cat, BP_FirstPersonCharacter | nej |
| `23okt/HavskattenSiaMeow4.uasset` | SoundWave | nej | SC_Cat | nej |
| `23okt/HavskattenSiaMeow5.uasset` | SoundWave | nej | SC_Cat | nej |
| `23okt/HavskattenSiaMiau.uasset` | SoundWave | nej | SC_Cat | nej |
| `23okt/HavskattenSiaSpinner.uasset` | SoundWave | nej | nej | ja |
| `23okt/HavskattenSoundMenu.uasset` | SoundWave | nej | nej | ja |
| `23okt/HavskattenSoundMenuAmbience.uasset` | SoundWave | nej | WBP_AudioOptions | nej |
| `23okt/HavskattenSoundMenuDialogueOnboarding.uasset` | SoundWave | nej | nej | ja |
| `23okt/HavskattenSoundMenuMaster.uasset` | SoundWave | nej | WBP_AudioOptions | nej |
| `23okt/HavskattenSoundMenuMusic.uasset` | SoundWave | nej | WBP_AudioOptions | nej |
| `23okt/HavskattenSoundMenuSFX.uasset` | SoundWave | nej | WBP_AudioOptions | nej |
| `23okt/HavskattenSoundMenuSliderEight.uasset` | SoundWave | nej | WBP_AudioOptions | nej |
| `23okt/HavskattenSoundMenuSliderFive.uasset` | SoundWave | nej | WBP_AudioOptions | nej |
| `23okt/HavskattenSoundMenuSliderFour.uasset` | SoundWave | nej | WBP_AudioOptions | nej |
| `23okt/HavskattenSoundMenuSliderNine.uasset` | SoundWave | nej | WBP_AudioOptions | nej |
| `23okt/HavskattenSoundMenuSliderOne.uasset` | SoundWave | nej | WBP_AudioOptions | nej |
| `23okt/HavskattenSoundMenuSliderSeven.uasset` | SoundWave | nej | WBP_AudioOptions | nej |
| `23okt/HavskattenSoundMenuSliderSix.uasset` | SoundWave | nej | WBP_AudioOptions | nej |
| `23okt/HavskattenSoundMenuSliderTen.uasset` | SoundWave | nej | nej | ja |
| `23okt/HavskattenSoundMenuSliderTenMax.uasset` | SoundWave | nej | WBP_AudioOptions | nej |
| `23okt/HavskattenSoundMenuSliderThree.uasset` | SoundWave | nej | WBP_AudioOptions | nej |
| `23okt/HavskattenSoundMenuSliderTwo.uasset` | SoundWave | nej | WBP_AudioOptions | nej |
| `23okt/HavskattenSoundMenuSliderZero.uasset` | SoundWave | nej | WBP_AudioOptions | nej |
| `23okt/HavskattenSoundMenuSliderZeroMin.uasset` | SoundWave | nej | nej | ja |
| `23okt/HavskattenStarGame01.uasset` | SoundWave | nej | WBP_MainMenu | nej |
| `23okt/HavskattenStartGame.uasset` | SoundWave | nej | WBP_MainMenu | nej |
| `23okt/Sia_Tagning1.uasset` | SoundWave | nej | nej | ja |
| `26okt/DuKanÖppnaShoppen.uasset` | SoundWave | nej | nej | ja |
| `26okt/IdittInventory.uasset` | SoundWave | nej | BP_FirstPersonCharacter | nej |
| `26okt/MytOmSkattkista.uasset` | SoundWave | nej | BP_FirstPersonCharacter | nej |
| `26okt/NavigeraMedVänster.uasset` | SoundWave | nej | BP_FirstPersonCharacter | nej |
| `26okt/NärDuHörFiskstim.uasset` | SoundWave | nej | BP_FirstPersonCharacter | nej |
| `26okt/SlutaVeva.uasset` | SoundWave | nej | BP_FirstPersonCharacter | nej |
| `26okt/SläppNedFiskespöet.uasset` | SoundWave | nej | BP_FirstPersonCharacter | nej |
| `26okt/TillEttSuperSpö.uasset` | SoundWave | nej | nej | ja |
| `26okt/TryckNerFörAtt3.uasset` | SoundWave | nej | BP_FirstPersonCharacter | nej |
| `26okt/TryckUppPåDPadFörAttKöpa.uasset` | SoundWave | nej | BP_FirstPersonCharacter | nej |
| `26okt/UppgraderaDittSpö.uasset` | SoundWave | nej | BP_FirstPersonCharacter | nej |
| `Attenuations/ATT_ReelSplash.uasset` | SoundAttenuation | nej | SC_Splash_Water_Small1_Cue | ja (bara via oanvänt ljud: SC_Splash_Water_Small1_Cue) |
| `Attenuations/ATT_Shop.uasset` | SoundAttenuation | nej | SC_Shop, Lvl_Joche (extern actor), Level_ShopTest (extern actor), Lvl_Cat (extern actor) (+1 till) | nej |
| `AudioPreMixed/BoatLoopLonger.uasset` | SoundWave | nej | SC_BoatLonger | nej |
| `AudioPreMixed/BoatLoopLow.uasset` | SoundWave | nej | SC_Boat | nej |
| `AudioPreMixed/FishConfirm.uasset` | SoundWave | nej | SC_CaughtFish | ja (bara via oanvänt ljud: SC_CaughtFish) |
| `AudioPreMixed/FishSplash01Mixed.uasset` | SoundWave | nej | nej | ja |
| `AudioPreMixed/FishSplash02Mixed.uasset` | SoundWave | nej | nej | ja |
| `AudioPreMixed/VictoryFanfare.uasset` | SoundWave | nej | Level_ShopTest, Lvl_Cat, Lvl_FirstPerson, Lvl_FirstPerson1 (+1 till) | nej |
| `Mix/SC_Ambience.uasset` | SoundClass | DefaultGame.ini | SC_Master, SM_Volume, ChimesWind_AMA01_67, LoggingTree_1016_77 (+17 till) | nej |
| `Mix/SC_Dialogue.uasset` | SoundClass | DefaultGame.ini | FiskebodBuyRod, FiskebodBuyRodShiort, FiskebodSellFishShort01, HavskattenFiskebodBuyFish (+80 till) | nej |
| `Mix/SC_Master.uasset` | SoundClass | DefaultGame.ini | SC_Ambience, SC_Dialogue, SC_Music, SC_SFX (+1 till) | nej |
| `Mix/SC_Music.uasset` | SoundClass | DefaultGame.ini | SC_Master, SM_Volume | nej |
| `Mix/SC_SFX.uasset` | SoundClass | DefaultGame.ini | SC_Master, SM_Volume, FishingReel_HB01_44_1, FishingReel_HB01_44_2 (+32 till) | nej |
| `Mix/SC_UIAccessibility.uasset` | SoundClass | nej | nej | ja |
| `Mix/SM_Volume.uasset` | SoundMix | DefaultGame.ini | nej | nej |
| `Mix/Submix_Ambience.uasset` | SoundSubmix | nej | nej | ja |
| `Mix/Submix_Dialogue.uasset` | SoundSubmix | nej | nej | ja |
| `Mix/Submix_Music.uasset` | SoundSubmix | nej | nej | ja |
| `Mix/Submix_SFX.uasset` | SoundSubmix | nej | nej | ja |
| `RawAudio/ChestEmerging_SetDownThud.uasset` | SoundWave | nej | BP_FirstPersonCharacter | nej |
| `RawAudio/ChimesWind_AMA01_67.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/FishingReel_HB01_44_1.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/FishingReel_HB01_44_2.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/Fishing_BaitSplash2.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/Fishing_PoleCastLine1.uasset` | SoundWave | nej | BP_FirstPersonCharacter | nej |
| `RawAudio/Fishing_PoleCastLine2.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/Fishing_PoleCastLine3.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/Fishing_PoleCastLine4.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/Fishing_PoleCastLine5.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/Fishing_PoleCastLine6.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/Fishing_PoleCastLine7.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/Fishing_PoleSwish.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/Fishing_ReelCrank_Loop1.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/Fishing_ReelCrank_Loop2.uasset` | SoundWave | nej | SC_Reeling | nej |
| `RawAudio/fisH_Caught.uasset` | SoundWave | nej | SC_FishCaught | nej |
| `RawAudio/fisH_lost.uasset` | SoundWave | nej | SC_FishLost | nej |
| `RawAudio/fisH_ROPE.uasset` | SoundWave | nej | SC_FishFighting | nej |
| `RawAudio/fisH_RopePull.uasset` | SoundWave | nej | SC_FishTension | nej |
| `RawAudio/FootstepBoot_FS03_45.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/FootstepsWood_6017_06_2.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/HavskattenQuit.uasset` | SoundWave | nej | WBP_MainMenu | nej |
| `RawAudio/HavskattenSettings.uasset` | SoundWave | nej | WBP_MainMenu | nej |
| `RawAudio/HavskattenSiaMeow2.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/HavskattenStartGame.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/LineReel01.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/LoggingTree_1016_77.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/loop_bubbles_1.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/MoneyBag_WB04_99_2.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/MoneyCashRegister_PS03_64.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/Röding.uasset` | SoundWave | nej (bara namnet i FishTypes.h, PauseMenu.cpp) | nej | ja |
| `RawAudio/SC_Splash_Water_Small1_Cue.uasset` | SoundCue | nej | nej | ja |
| `RawAudio/SeaGull01mono.uasset` | SoundWave | nej | SC_SeaGull | nej |
| `RawAudio/SeaGull01stereo.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/SeaGull02mono.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/SeaGull02stereo.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/Sonar_High01.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/Sonar_Low01.uasset` | SoundWave | nej | SC_Sonar | nej |
| `RawAudio/splash-loop-93450.uasset` | SoundWave | Fish.cpp | nej | nej |
| `RawAudio/Splash_Water_Small1.uasset` | SoundWave | nej | SC_Splash_Water_Small1_Cue, BP_FirstPersonCharacter | nej |
| `RawAudio/Splash_Water_Small2.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/WaterLake.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/WaterLake_6035_07.uasset` | SoundWave | nej | SC_AmbeinceLake | nej |
| `RawAudio/WaterSplash_1018_46.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/WaterSplash_6100_19_2.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/WeatherWind_CTV06_93_01.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/WindTrees_IMM07_08.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/WindTrees_IMM07_09.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/WindTrees_IMM07_10.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/WindTrees_IMM07_11.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/WindTrees_IMM07_12.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/WindTrees_IMM07_13.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/FiskInfo/Aborre.uasset` | SoundWave | nej (bara namnet i FishTypes.h, PauseMenu.cpp) | BP_FishManager, BP_FirstPersonCharacter | nej |
| `RawAudio/FiskInfo/Arton.uasset` | SoundWave | nej | BP_FirstPersonCharacter, Lvl_FirstPerson (extern actor) | nej |
| `RawAudio/FiskInfo/AvanceratSpö.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/FiskInfo/Braxen.uasset` | SoundWave | nej (bara namnet i FishTypes.h, PauseMenu.cpp) | BP_FishManager, BP_FirstPersonCharacter, Lvl_FirstPerson1 (extern actor) | nej |
| `RawAudio/FiskInfo/Elva.uasset` | SoundWave | nej | BP_FirstPersonCharacter, Lvl_FirstPerson (extern actor) | nej |
| `RawAudio/FiskInfo/Ett.uasset` | SoundWave | nej | BP_FirstPersonCharacter, Lvl_FirstPerson (extern actor) | nej |
| `RawAudio/FiskInfo/EttHundra.uasset` | SoundWave | nej | Lvl_FirstPerson (extern actor), Lvl_FirstPerson1 (extern actor) | nej |
| `RawAudio/FiskInfo/Fem.uasset` | SoundWave | nej | BP_FirstPersonCharacter, Lvl_FirstPerson (extern actor), Lvl_FirstPerson1 (extern actor) | nej |
| `RawAudio/FiskInfo/Femti.uasset` | SoundWave | nej | BP_FirstPersonCharacter, Lvl_FirstPerson (extern actor) | nej |
| `RawAudio/FiskInfo/Femtio.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/FiskInfo/Femton.uasset` | SoundWave | nej | BP_FirstPersonCharacter, Lvl_FirstPerson (extern actor) | nej |
| `RawAudio/FiskInfo/Fjorton.uasset` | SoundWave | nej | BP_FirstPersonCharacter, Lvl_FirstPerson (extern actor) | nej |
| `RawAudio/FiskInfo/Fyra.uasset` | SoundWave | nej | BP_FirstPersonCharacter, Lvl_FirstPerson (extern actor) | nej |
| `RawAudio/FiskInfo/Fyrti.uasset` | SoundWave | nej | BP_FirstPersonCharacter, Lvl_FirstPerson (extern actor) | nej |
| `RawAudio/FiskInfo/Fyrtio.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/FiskInfo/Gädda.uasset` | SoundWave | nej (bara namnet i FishTypes.h, PauseMenu.cpp) | BP_FishManager, BP_FirstPersonCharacter | nej |
| `RawAudio/FiskInfo/Gös.uasset` | SoundWave | nej (bara namnet i FishTypes.h, PauseMenu.cpp) | BP_FishManager, BP_FirstPersonCharacter | nej |
| `RawAudio/FiskInfo/Hundra.uasset` | SoundWave | nej | BP_FirstPersonCharacter | nej |
| `RawAudio/FiskInfo/InsjöÖring.uasset` | SoundWave | nej | BP_FishManager, BP_FirstPersonCharacter | nej |
| `RawAudio/FiskInfo/Kilo.uasset` | SoundWave | nej | BP_FirstPersonCharacter | nej |
| `RawAudio/FiskInfo/Kilogram.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/FiskInfo/Kronor.uasset` | SoundWave | nej | BP_FirstPersonCharacter | nej |
| `RawAudio/FiskInfo/Lake.uasset` | SoundWave | nej (bara namnet i FishTypes.h, PauseMenu.cpp) | BP_FishManager, BP_FirstPersonCharacter | nej |
| `RawAudio/FiskInfo/Nio.uasset` | SoundWave | nej | BP_FirstPersonCharacter, Lvl_FirstPerson (extern actor) | nej |
| `RawAudio/FiskInfo/Nitti.uasset` | SoundWave | nej | BP_FirstPersonCharacter, Lvl_FirstPerson (extern actor) | nej |
| `RawAudio/FiskInfo/Nittio.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/FiskInfo/Nitton.uasset` | SoundWave | nej | BP_FirstPersonCharacter, Lvl_FirstPerson (extern actor) | nej |
| `RawAudio/FiskInfo/Noll.uasset` | SoundWave | nej | Lvl_FirstPerson (extern actor) | nej |
| `RawAudio/FiskInfo/Regnbågslax.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/FiskInfo/Regnbåge.uasset` | SoundWave | nej (bara namnet i FishTypes.h, PauseMenu.cpp) | BP_FishManager, BP_FirstPersonCharacter | nej |
| `RawAudio/FiskInfo/Regnbågslax.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/FiskInfo/Röding.uasset` | SoundWave | nej (bara namnet i FishTypes.h, PauseMenu.cpp) | BP_FishManager, BP_FirstPersonCharacter, Lvl_FirstPerson1 (extern actor) | nej |
| `RawAudio/FiskInfo/Röding.uasset` | SoundWave | nej (bara namnet i FishTypes.h, PauseMenu.cpp) | BP_FishManager, BP_FirstPersonCharacter, Lvl_FirstPerson1 (extern actor) | nej |
| `RawAudio/FiskInfo/Sex.uasset` | SoundWave | nej | BP_FirstPersonCharacter, Lvl_FirstPerson (extern actor) | nej |
| `RawAudio/FiskInfo/Sexti.uasset` | SoundWave | nej | BP_FirstPersonCharacter, Lvl_FirstPerson (extern actor) | nej |
| `RawAudio/FiskInfo/Sextio.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/FiskInfo/Sexton.uasset` | SoundWave | nej | BP_FirstPersonCharacter, Lvl_FirstPerson (extern actor) | nej |
| `RawAudio/FiskInfo/Sik.uasset` | SoundWave | nej (bara namnet i FishTypes.h, PauseMenu.cpp) | BP_FishManager, BP_FirstPersonCharacter | nej |
| `RawAudio/FiskInfo/Sju.uasset` | SoundWave | nej | BP_FirstPersonCharacter, Lvl_FirstPerson (extern actor) | nej |
| `RawAudio/FiskInfo/Sjuti.uasset` | SoundWave | nej | BP_FirstPersonCharacter, Lvl_FirstPerson (extern actor) | nej |
| `RawAudio/FiskInfo/Sjutio.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/FiskInfo/Sjutton.uasset` | SoundWave | nej | BP_FirstPersonCharacter, Lvl_FirstPerson (extern actor) | nej |
| `RawAudio/FiskInfo/StandardSpö.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/FiskInfo/Startspö.uasset` | SoundWave | nej (bara namnet i FishTypes.h) | nej | ja |
| `RawAudio/FiskInfo/SuperSpö.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/FiskInfo/Sutare.uasset` | SoundWave | nej (bara namnet i FishTypes.h, Inventory.cpp, PauseMenu.cpp) | BP_FishManager, BP_FirstPersonCharacter | nej |
| `RawAudio/FiskInfo/Tio.uasset` | SoundWave | nej | BP_FirstPersonCharacter, Lvl_FirstPerson (extern actor) | nej |
| `RawAudio/FiskInfo/Tjugi.uasset` | SoundWave | nej | BP_FirstPersonCharacter, Lvl_FirstPerson (extern actor) | nej |
| `RawAudio/FiskInfo/TOlv.uasset` | SoundWave | nej | BP_FirstPersonCharacter, Lvl_FirstPerson (extern actor) | nej |
| `RawAudio/FiskInfo/Tre.uasset` | SoundWave | nej | BP_FirstPersonCharacter, Lvl_FirstPerson (extern actor) | nej |
| `RawAudio/FiskInfo/Tretti.uasset` | SoundWave | nej | BP_FirstPersonCharacter, Lvl_FirstPerson (extern actor) | nej |
| `RawAudio/FiskInfo/Trettio.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/FiskInfo/Tretton.uasset` | SoundWave | nej | BP_FirstPersonCharacter, Lvl_FirstPerson (extern actor) | nej |
| `RawAudio/FiskInfo/Två.uasset` | SoundWave | nej | BP_FirstPersonCharacter, Lvl_FirstPerson (extern actor) | nej |
| `RawAudio/FiskInfo/Åtta.uasset` | SoundWave | nej | BP_FirstPersonCharacter, Lvl_FirstPerson (extern actor) | nej |
| `RawAudio/FiskInfo/Åtti.uasset` | SoundWave | nej | BP_FirstPersonCharacter, Lvl_FirstPerson (extern actor) | nej |
| `RawAudio/FiskInfo/Åttio.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/FiskInfo/ÅttiSjutio.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/Menu/Avsluta.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/Menu/HuvudMeny.uasset` | SoundWave | nej | WBP_AudioOptions | nej |
| `RawAudio/Menu/LjudInst.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/Menu/StartaSpel.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/Onboarding/HavskattenIntro1.uasset` | SoundWave | nej | BP_FirstPersonCharacter | nej |
| `RawAudio/Onboarding/HavskattenIntro2.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/Onboarding/HavskattenIntro3.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/Onboarding/HavskattenIntro4.uasset` | SoundWave | nej | BP_FirstPersonCharacter | nej |
| `RawAudio/Onboarding/HavskattenIntro5.uasset` | SoundWave | nej | BP_FirstPersonCharacter | nej |
| `RawAudio/Onboarding/HavskattenIntro6.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/Onboarding/HavskattenIntro6RTVersion.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/Onboarding/HavskattenIntro7.uasset` | SoundWave | nej | BP_FirstPersonCharacter | nej |
| `RawAudio/Onboarding/HavskattenIntro8.uasset` | SoundWave | nej | BP_FirstPersonCharacter | nej |
| `RawAudio/Onboarding/HavskattenIntro9.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/Onboarding/HavskattenIntro_Where_Right_Trigger.uasset` | SoundWave | nej | nej | ja |
| `RawAudio/Onboarding/HavskattenWelcome.uasset` | SoundWave | nej | BP_FirstPersonCharacter | nej |
| `SoundAttenuation/AT_SeagullAttenuation.uasset` | SoundAttenuation | nej | SC_SeaGull | nej |
| `SoundCues/SC_AmbeinceLake.uasset` | SoundCue | nej | Lvl_Joche (extern actor), Level_ShopTest (extern actor), Lvl_Cat (extern actor), Lvl_FirstPerson (extern actor) (+2 till) | nej |
| `SoundCues/SC_AmbeinceLand.uasset` | SoundCue | nej | Lvl_Joche (extern actor), Level_ShopTest (extern actor), Lvl_Cat (extern actor), Lvl_FirstPerson (extern actor) (+2 till) | nej |
| `SoundCues/SC_Boat.uasset` | SoundCue | nej | Lvl_Joche (extern actor), Level_ShopTest (extern actor), Lvl_Cat (extern actor), Lvl_FirstPerson (extern actor) (+2 till) | nej |
| `SoundCues/SC_BoatLonger.uasset` | SoundCue | nej | BP_FirstPersonCharacter | nej |
| `SoundCues/SC_Cat.uasset` | SoundCue | nej | BP_Cat | nej |
| `SoundCues/SC_CaughtFish.uasset` | SoundCue | nej | nej | ja |
| `SoundCues/SC_Dock.uasset` | SoundCue | nej | Lvl_Joche (extern actor), Level_ShopTest (extern actor), Lvl_Cat (extern actor), Lvl_FirstPerson (extern actor) (+2 till) | nej |
| `SoundCues/SC_FishCaught.uasset` | SoundCue | nej | BP_FirstPersonCharacter | nej |
| `SoundCues/SC_FishFighting.uasset` | SoundCue | nej | BP_FirstPersonCharacter | nej |
| `SoundCues/SC_Fishing.uasset` | SoundCue | nej | nej | ja |
| `SoundCues/SC_FishLost.uasset` | SoundCue | nej | BP_FirstPersonCharacter | nej |
| `SoundCues/SC_FishTension.uasset` | SoundCue | nej | BP_FirstPersonCharacter | nej |
| `SoundCues/SC_Reeling.uasset` | SoundCue | nej | BP_FirstPersonCharacter | nej |
| `SoundCues/SC_SeaGull.uasset` | SoundCue | Fish.cpp | Level_ShopTest (extern actor), Lvl_Cat (extern actor), Lvl_FirstPerson (extern actor), Lvl_FirstPerson1 (extern actor) (+1 till) | nej |
| `SoundCues/SC_Shop.uasset` | SoundCue | nej | BP_Shop, Lvl_Joche (extern actor), Level_ShopTest (extern actor), Lvl_Cat (extern actor) (+3 till) | nej |
| `SoundCues/SC_Sonar.uasset` | SoundCue | nej | BP_FirstPersonCharacter | nej |

## Bara de troligen oanvända

- `SeaGull01mono.uasset` (SoundWave)
- `SeagullStereo.uasset` (SoundWave)
- `23okt/FiskebodBuyRod.uasset` (SoundWave)
- `23okt/FiskebodBuyRodShiort.uasset` (SoundWave)
- `23okt/FiskebodSellFishShort01.uasset` (SoundWave)
- `23okt/HavskattenFiskebodBuyFish.uasset` (SoundWave)
- `23okt/HavskattenFiskebodBuyRod01.uasset` (SoundWave)
- `23okt/HavskattenFiskebodBuyRodShort01.uasset` (SoundWave)
- `23okt/HavskattenFiskebodFramme_Tagn_1.uasset` (SoundWave)
- `23okt/HavskattenFiskebodFramme_Tagn_2.uasset` (SoundWave)
- `23okt/HavskattenFiskebodSellFish.uasset` (SoundWave)
- `23okt/HavskattenFiskebodSellFishShort.uasset` (SoundWave)
- `23okt/HavskattenFiskebodSellFishShort02.uasset` (SoundWave)
- `23okt/HavskattenFiskebodSellFishShort03.uasset` (SoundWave)
- `23okt/HavskattenIntroNavigation.uasset` (SoundWave)
- `23okt/HavskattenQuit.uasset` (SoundWave)
- `23okt/HavskattenSettings.uasset` (SoundWave)
- `23okt/HavskattenSettings1.uasset` (SoundWave)
- `23okt/HavskattenSettingsSound.uasset` (SoundWave)
- `23okt/HavskattenSettingsSoundAmbience.uasset` (SoundWave)
- `23okt/HavskattenSettingsSoundMaster.uasset` (SoundWave)
- `23okt/HavskattenSettingsSoundMusic.uasset` (SoundWave)
- `23okt/HavskattenSettingsSoundSFX.uasset` (SoundWave)
- `23okt/HavskattenSiaSpinner.uasset` (SoundWave)
- `23okt/HavskattenSoundMenu.uasset` (SoundWave)
- `23okt/HavskattenSoundMenuDialogueOnboarding.uasset` (SoundWave)
- `23okt/HavskattenSoundMenuSliderTen.uasset` (SoundWave)
- `23okt/HavskattenSoundMenuSliderZeroMin.uasset` (SoundWave)
- `23okt/Sia_Tagning1.uasset` (SoundWave)
- `26okt/DuKanÖppnaShoppen.uasset` (SoundWave)
- `26okt/TillEttSuperSpö.uasset` (SoundWave)
- `Attenuations/ATT_ReelSplash.uasset` (SoundAttenuation)
- `AudioPreMixed/FishConfirm.uasset` (SoundWave)
- `AudioPreMixed/FishSplash01Mixed.uasset` (SoundWave)
- `AudioPreMixed/FishSplash02Mixed.uasset` (SoundWave)
- `Mix/SC_UIAccessibility.uasset` (SoundClass)
- `Mix/Submix_Ambience.uasset` (SoundSubmix)
- `Mix/Submix_Dialogue.uasset` (SoundSubmix)
- `Mix/Submix_Music.uasset` (SoundSubmix)
- `Mix/Submix_SFX.uasset` (SoundSubmix)
- `RawAudio/ChimesWind_AMA01_67.uasset` (SoundWave)
- `RawAudio/FishingReel_HB01_44_1.uasset` (SoundWave)
- `RawAudio/FishingReel_HB01_44_2.uasset` (SoundWave)
- `RawAudio/Fishing_BaitSplash2.uasset` (SoundWave)
- `RawAudio/Fishing_PoleCastLine2.uasset` (SoundWave)
- `RawAudio/Fishing_PoleCastLine3.uasset` (SoundWave)
- `RawAudio/Fishing_PoleCastLine4.uasset` (SoundWave)
- `RawAudio/Fishing_PoleCastLine5.uasset` (SoundWave)
- `RawAudio/Fishing_PoleCastLine6.uasset` (SoundWave)
- `RawAudio/Fishing_PoleCastLine7.uasset` (SoundWave)
- `RawAudio/Fishing_PoleSwish.uasset` (SoundWave)
- `RawAudio/Fishing_ReelCrank_Loop1.uasset` (SoundWave)
- `RawAudio/FootstepBoot_FS03_45.uasset` (SoundWave)
- `RawAudio/FootstepsWood_6017_06_2.uasset` (SoundWave)
- `RawAudio/HavskattenSiaMeow2.uasset` (SoundWave)
- `RawAudio/HavskattenStartGame.uasset` (SoundWave)
- `RawAudio/LineReel01.uasset` (SoundWave)
- `RawAudio/LoggingTree_1016_77.uasset` (SoundWave)
- `RawAudio/loop_bubbles_1.uasset` (SoundWave)
- `RawAudio/MoneyBag_WB04_99_2.uasset` (SoundWave)
- `RawAudio/MoneyCashRegister_PS03_64.uasset` (SoundWave)
- `RawAudio/Röding.uasset` (SoundWave)
- `RawAudio/SC_Splash_Water_Small1_Cue.uasset` (SoundCue)
- `RawAudio/SeaGull01stereo.uasset` (SoundWave)
- `RawAudio/SeaGull02mono.uasset` (SoundWave)
- `RawAudio/SeaGull02stereo.uasset` (SoundWave)
- `RawAudio/Sonar_High01.uasset` (SoundWave)
- `RawAudio/Splash_Water_Small2.uasset` (SoundWave)
- `RawAudio/WaterLake.uasset` (SoundWave)
- `RawAudio/WaterSplash_1018_46.uasset` (SoundWave)
- `RawAudio/WaterSplash_6100_19_2.uasset` (SoundWave)
- `RawAudio/WeatherWind_CTV06_93_01.uasset` (SoundWave)
- `RawAudio/WindTrees_IMM07_08.uasset` (SoundWave)
- `RawAudio/WindTrees_IMM07_09.uasset` (SoundWave)
- `RawAudio/WindTrees_IMM07_10.uasset` (SoundWave)
- `RawAudio/WindTrees_IMM07_11.uasset` (SoundWave)
- `RawAudio/WindTrees_IMM07_12.uasset` (SoundWave)
- `RawAudio/WindTrees_IMM07_13.uasset` (SoundWave)
- `RawAudio/FiskInfo/AvanceratSpö.uasset` (SoundWave)
- `RawAudio/FiskInfo/Femtio.uasset` (SoundWave)
- `RawAudio/FiskInfo/Fyrtio.uasset` (SoundWave)
- `RawAudio/FiskInfo/Kilogram.uasset` (SoundWave)
- `RawAudio/FiskInfo/Nittio.uasset` (SoundWave)
- `RawAudio/FiskInfo/Regnbågslax.uasset` (SoundWave)
- `RawAudio/FiskInfo/Regnbågslax.uasset` (SoundWave)
- `RawAudio/FiskInfo/Sextio.uasset` (SoundWave)
- `RawAudio/FiskInfo/Sjutio.uasset` (SoundWave)
- `RawAudio/FiskInfo/StandardSpö.uasset` (SoundWave)
- `RawAudio/FiskInfo/Startspö.uasset` (SoundWave)
- `RawAudio/FiskInfo/SuperSpö.uasset` (SoundWave)
- `RawAudio/FiskInfo/Trettio.uasset` (SoundWave)
- `RawAudio/FiskInfo/Åttio.uasset` (SoundWave)
- `RawAudio/FiskInfo/ÅttiSjutio.uasset` (SoundWave)
- `RawAudio/Menu/Avsluta.uasset` (SoundWave)
- `RawAudio/Menu/LjudInst.uasset` (SoundWave)
- `RawAudio/Menu/StartaSpel.uasset` (SoundWave)
- `RawAudio/Onboarding/HavskattenIntro2.uasset` (SoundWave)
- `RawAudio/Onboarding/HavskattenIntro3.uasset` (SoundWave)
- `RawAudio/Onboarding/HavskattenIntro6.uasset` (SoundWave)
- `RawAudio/Onboarding/HavskattenIntro6RTVersion.uasset` (SoundWave)
- `RawAudio/Onboarding/HavskattenIntro9.uasset` (SoundWave)
- `RawAudio/Onboarding/HavskattenIntro_Where_Right_Trigger.uasset` (SoundWave)
- `SoundCues/SC_CaughtFish.uasset` (SoundCue)
- `SoundCues/SC_Fishing.uasset` (SoundCue)
