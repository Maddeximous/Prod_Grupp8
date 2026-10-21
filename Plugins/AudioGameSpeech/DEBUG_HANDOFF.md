# Handoff: Piper speech silent on a teammate's PC

For Claude Code on the teammate's PC. Read this first, then `README.md` in this folder.

## The problem
- Plugin `Plugins/AudioGameSpeech` (UE 5.8, branch `feature/speech-core`) speaks Swedish with Piper.
- **It works on the main dev PC but not on this PC: no speech is heard.**
- Other game sounds ARE heard on this PC (same output: "Headphones (High Definition Audio Device)").
- The user can only see ~1500 characters per reply, so keep replies short.

## How speech works
- `USpeechSubsystem` (GameInstance subsystem) -> backend `PiperSpeechBackend` runs hidden
  `ThirdParty/Piper/piper.exe` with `voices/sv_SE-nst-medium.onnx`, gets back a 16-bit PCM WAV.
- `PlayClip()` in `Source/AudioGameSpeech/Private/SpeechSubsystem.cpp` puts the PCM in a
  `USoundWaveProcedural` and plays it with `UGameplayStatics::CreateSound2D` (2D, UI sound, volume from `SetVolume`).
- A ticker ends the line after `Clip.Duration` (procedural sounds never end by themselves).
- Test actor: `BP_SpeechTest` (`Content/ScreenReader/`, C++ `SpeechTestActor.cpp`) speaks on BeginPlay.

## What has already been checked (log from this PC, 2026-10-07)
- Plugin DLL loads, `Speech ready. Voice: Piper nst (sv-SE)`, `Piper started`.
- `Speech started (9.42 s)`, then `Speech finished`, then the next line starts. No errors or warnings.
- Audio device is created fine and is the same one used by the game sounds.
- So: Piper returns audio of the right length and Unreal is told to play it, but nothing is heard.

## Recent changes (already pushed)
- Piper exe, DLLs, `espeak-ng-data/` and the voice are now **committed to git** (no setup needed).
  `ThirdParty/Piper/.gitattributes` has `* -text` so line endings stay unchanged.
- `Setup-Piper.ps1` is now only for repairs (re-downloads if `piper.exe` or `espeak-ng-data` is missing).

## New diagnostic (in this commit, NOT yet compiled anywhere)
`DebugDumpClip()` in `SpeechSubsystem.cpp`, called at the start of `PlayClip()`:
- Logs `LogAudioGameSpeech: Clip: <Hz> Hz, <ch> ch, <bytes> bytes, peak <N>%`
- Saves the clip to `Saved/Speech/LastSpeech.wav`
If it does not compile, fix it (it was written without a build).

## Steps to do on this PC
1. Pull `feature/speech-core`. Close the editor, build (`Prod_grupp8Editor Win64 Development`), open the editor.
2. Check the Piper files are complete and unchanged:
   `ThirdParty/Piper/espeak-ng-data/phontab` exists, `voices/sv_SE-nst-medium.onnx` is 63,104,526 bytes,
   `piper.exe` SHA256 = `96f3da3811151580073e40bb4dd20eb0fb8115f5f5f76e2fb54282b3edfa5c1f`.
   Run `git status` in `ThirdParty/Piper` - there should be no changes.
3. Test Piper outside Unreal (from `ThirdParty/Piper`):
   `echo Hej, det här är ett test. | piper.exe -m voices/sv_SE-nst-medium.onnx -f test.wav`
   then play `test.wav`. Audible -> Piper is fine.
4. Put `BP_SpeechTest` in a level, press Play, click inside the game window.
5. Read `Saved/Logs/Prod_grupp8.log` (`grep LogAudioGameSpeech`) and play `Saved/Speech/LastSpeech.wav`.

## How to read the result
- **peak ~0% / LastSpeech.wav silent** -> Piper gives silence on this PC. Compare with step 3; check
  espeak-ng-data, the voice file, antivirus blocking `piper.exe`, path with special characters.
- **peak is normal (e.g. 30-90%) and LastSpeech.wav is audible, but nothing in game** -> Unreal playback problem.
  Ideas: sample rate (Piper is 22050 Hz mono) on this audio device, `USoundWaveProcedural` setup
  (try `Wave->bProcedural`, `Wave->SoundGroup = SOUNDGROUP_Voice`, or setting `Wave->bCanProcessAsync`),
  voice limit (32 channels; try `ActiveComponent->Priority`/`bIsUISound`), editor volume / "mute when unfocused",
  Windows volume mixer for UnrealEditor.
  Quick test: import `LastSpeech.wav` as a normal Sound Wave and play it in the level. If that works,
  the bug is in how `PlayClip()` builds the procedural wave.

## Rules
- Keep all engines behind `ISpeechBackend`. Piper stays first in the fallback order.
- No Git LFS. Every file must be under 100 MB.
- Remove `DebugDumpClip` (or put it behind a flag) once the bug is found.
- Commit on `feature/speech-core`. The user merges into `main` themselves.
