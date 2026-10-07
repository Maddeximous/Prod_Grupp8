# Audio Game Speech

Text-to-speech (TTS) for our audio game. All synthesized speech in the game (menus,
cutscene descriptions, gameplay announcements) goes through this plugin.

- **Default voice: Piper.** Offline Swedish voice shipped with the game. Works the
  moment the game is installed, no Windows setup needed.
- **Optional: Windows voices** (Microsoft Bengt). Later also third-party SAPI5 voices
  and players' own APIs.
- Speech is turned into audio and played **through Unreal**, so we control timing,
  volume, speed, ducking and pause behaviour.
- Windows only (Win64).
- Recorded voice lines do NOT use this. They are normal sound assets.

---

## Contents

1. [Quick start](#1-quick-start)
2. [Using it from Blueprints](#2-using-it-from-blueprints)
3. [Using it from C++](#3-using-it-from-c)
4. [API reference](#4-api-reference)
5. [How it works](#5-how-it-works)
6. [Queue and cache](#6-queue-and-cache)
7. [Engines (backends)](#7-engines-backends)
8. [Piper in detail](#8-piper-in-detail)
9. [Packaging](#9-packaging)
10. [Adding a new engine](#10-adding-a-new-engine)
11. [Troubleshooting](#11-troubleshooting)
12. [Known limits and planned work](#12-known-limits-and-planned-work)
13. [Rules](#13-rules)

---

## 1. Quick start

**After cloning the repo (once per computer):**

```
powershell -ExecutionPolicy Bypass -File Plugins/AudioGameSpeech/ThirdParty/Piper/Setup-Piper.ps1
```

This downloads `piper.exe`, its DLLs, `espeak-ng-data/` and the Swedish voice
`sv_SE-nst-medium` (~60 MB) into `ThirdParty/Piper/`. These files are **git-ignored**
(binaries and too big for git), so every coder has to run the script.

Then build the project as usual. The plugin is already enabled in `Prod_grupp8.uproject`.

**Test it:** place `BP_SpeechTest` (in `Content/ScreenReader/`) in a level and press Play.
The Output Log should show:

```
LogAudioGameSpeech: Speech ready. Voice: Piper nst (sv-SE). Swedish voice installed: yes
LogAudioGameSpeech: Piper started: sv_SE-nst-medium, speed 1.00
```

---

## 2. Using it from Blueprints

Get the subsystem with the **Get SpeechSubsystem** node (it exists on the Game
Instance), drag off it and pick a function.

| Situation | What to do |
|---|---|
| Game start | `Precache` all menu texts. Create a "Speech" Sound Class and pass it to `Set Sound Class`. |
| Menu focus changes | `Speak(text, Interrupt = true)` |
| Gameplay announcement | `Speak(text, Interrupt = false)` (waits in line) |
| "Say again" button | `Repeat Last` (e.g. right stick click) |
| Settings sliders | `Set Speaking Rate` (1.0 = normal), `Set Volume` (0-1) |
| Voice picker | `Get Voices` to list, `Set Voice(Id)` to pick |
| Cutscenes | Use `On Speech Started` (gives exact duration) and `On Speech Finished` to advance |

---

## 3. Using it from C++

**1. Add the plugin as a dependency** of your module (e.g. `Source/Prod_grupp8/Prod_grupp8.Build.cs`):

```cs
PrivateDependencyModuleNames.AddRange(new string[] { "AudioGameSpeech" });
```

**2. Get the subsystem and call it:**

```cpp
// MyDoor.cpp
#include "MyDoor.h"
#include "SpeechSubsystem.h"
#include "Engine/GameInstance.h"

void AMyDoor::OnPlayerTriedDoor()
{
    UGameInstance* GI = GetGameInstance();
    USpeechSubsystem* Speech = GI ? GI->GetSubsystem<USpeechSubsystem>() : nullptr;
    if (!Speech)
    {
        return;
    }

    // false = wait in line, don't cut off current speech
    Speech->Speak(NSLOCTEXT("Door", "Locked", "Dörren är låst."), false);
}
```

**3. Listening to events** (e.g. a cutscene that advances when a line ends):

```cpp
// .h - the handler must be a UFUNCTION
UFUNCTION()
void HandleSpeechFinished();

// BeginPlay
Speech->OnSpeechFinished.AddDynamic(this, &AMyCutscene::HandleSpeechFinished);

// EndPlay - always unbind
Speech->OnSpeechFinished.RemoveDynamic(this, &AMyCutscene::HandleSpeechFinished);
```

See `SpeechTestActor.cpp` for a complete working example.

**Tips**

- Don't keep the pointer across levels. Fetching it is cheap, fetch when needed.
- Without an actor: `World->GetGameInstance()->GetSubsystem<USpeechSubsystem>()`.
- Use string tables for real game text instead of `NSLOCTEXT`, so it can be translated.

---

## 4. API reference

All in `Public/SpeechSubsystem.h`. "Calls" = things the game tells the speech system
to do. "Events" = things the speech system tells the game happened.

### Calls

| Function | What it does |
|---|---|
| `Speak(Text, bInterrupt = true)` | Say text. Interrupt = cut off current speech and clear the queue (menus). Otherwise queue after it (gameplay). |
| `StopSpeaking()` | Go quiet now and clear the queue. |
| `IsSpeaking()` | True if a line is playing, being rendered or waiting. |
| `RepeatLast()` | Say the last line again. |
| `Precache(Lines)` | Render lines now so they play instantly later. Re-rendered automatically when voice/speed changes. |
| `GetVoices(bSwedishOnly = true)` | All voices from all engines. |
| `SetVoice(VoiceId)` | Pick a voice. Switches engine if needed (old engine shuts down). |
| `GetCurrentVoice()` | The voice in use. |
| `HasSwedishVoice()` | False only if Piper is missing AND Windows has no Swedish voice. Then play a recorded help message. |
| `SetSpeakingRate(Rate)` | 1.0 = normal, range 0.5 to 6.0. |
| `SetVolume(Volume)` | 0 to 1. Also affects the line playing now. |
| `SetSoundClass(Class)` | Your "Speech" Sound Class, so volume settings and ducking apply. |

### Events

| Event | When |
|---|---|
| `OnSpeechStarted(DurationSeconds)` | A line starts. Duration is exact, use it for cutscene timing. |
| `OnSpeechFinished` | A line ended normally. **Not** fired when interrupted. |

It's safe to call `Speak` from inside these events.

### Types (`Public/SpeechTypes.h`)

- `FSpeechVoice`: `Id`, `DisplayName`, `Language` (e.g. `sv-SE`), `Engine`.
- `ESpeechEngine`: `WindowsOneCore`, `ThirdPartySAPI5` (not implemented), `Piper`.
  New values are added at the end so saved settings stay valid.

---

## 5. How it works

### File overview

```
AudioGameSpeech/
├─ AudioGameSpeech.uplugin
├─ ThirdParty/Piper/
│  ├─ Setup-Piper.ps1             Downloads Piper + Swedish voice (only this file is in git)
│  └─ (piper.exe, DLLs, espeak-ng-data/, voices/)   after running the script
└─ Source/AudioGameSpeech/
   ├─ AudioGameSpeech.Build.cs     Build settings, ships Piper files in packaged builds
   ├─ Public/
   │  ├─ SpeechSubsystem.h         THE entry point. Everything calls this.
   │  ├─ SpeechTypes.h             Voice struct, engine enum, audio clip
   │  └─ SpeechTestActor.h         Test actor
   └─ Private/
      ├─ SpeechSubsystem.cpp       Queue, cache, playback, settings, engine switching
      ├─ SpeechWorker.h/.cpp       Background thread that renders speech
      ├─ SpeechBackend.h           Interface every engine implements
      ├─ PiperSpeechBackend.*      Built-in voice: runs piper.exe hidden
      ├─ WinRTSpeechBackend.*      Windows voices (Bengt)
      ├─ Sapi5SpeechBackend.h      Placeholder + plan for third-party voices
      ├─ SpeechWav.h               WAV file -> raw audio
      └─ SpeechLog.h, *Module.cpp  Logging, module boilerplate
```

### Layers

```
 Game code / Blueprints / menus
            │  Speak, Precache, SetVoice ...        ▲ OnSpeechStarted / Finished
            ▼                                       │
 ┌─────────────────────── USpeechSubsystem (game thread) ───────────────────────┐
 │  Queue  ->  Cache hit? ── yes ──────────────────────────────> Play in Unreal  │
 │                 │ no                                          (2D UI sound,   │
 │                 ▼                                              Sound Class)  │
 └──────── FSpeechWorker (background thread) ───────── result back to game ─────┘
                   │  Synthesize(text, voice, rate)          thread -> cache -> play
                   ▼
          ISpeechBackend  (only the ACTIVE engine has a worker)
          ├─ FPiperSpeechBackend   -> piper.exe (child process)
          ├─ FWinRTSpeechBackend   -> Windows OneCore (Bengt)
          └─ (later) SAPI5, player APIs ...
```

1. **`USpeechSubsystem`** (Game Instance Subsystem): one per game session, survives
   level changes. Owns the queue, cache, settings and playback. The only thing the
   rest of the game talks to.
2. **`FSpeechWorker`**: a background thread. Turning text into audio takes time, so it
   never happens on the game thread. Jobs are processed in order. Cancelled jobs are
   skipped.
3. **`ISpeechBackend`**: small interface every engine implements:
   `GetVoices()`, `Prepare()`, `Synthesize()`, plus thread start/stop hooks.
4. **Playback**: the raw audio is put in a `USoundWaveProcedural` and played as a
   **2D UI sound**, so it keeps playing when the game is paused and across level
   changes. A timer stops it after its exact duration, then the next line starts.

### Startup (in `USpeechSubsystem::Initialize`)

1. Create all engines that are available (Piper if `ThirdParty/Piper/piper.exe` exists,
   Windows OneCore always).
2. Collect voices from all engines.
3. Pick the default voice: **Piper Swedish → Bengt → any Swedish → anything**.
4. Start the worker for that voice's engine.
5. Send a "prepare" job so Piper loads its model now (~1 s) instead of on the first line.

### Switching engine

`SetVoice` with a voice from another engine:
stop speaking → destroy old worker (its engine shuts down, piper.exe closes) →
start new worker → clear cache → re-render precached lines.
Only one engine runs at a time.

---

## 6. Queue and cache

### Queue: when to speak

Only one line plays at a time. New lines wait their turn. When a line ends, the next
starts automatically.

- **Interrupt on** (menus): stop the current line, clear the queue, say the new one.
  When the player scrolls fast they only hear where they land.
- **Interrupt off** (gameplay): add to the end. Nothing is cut off.
  E.g. "Du hittade en nyckel" → "Dörren är låst".
- Interrupting also cancels lines still being rendered in the background.

### Cache: making it fast

Rendering a line takes Piper a short moment. The result is kept in memory, so the
next time the same line is said it plays instantly.

- Key = voice + speed + text. Change voice or speed and the old audio is wrong, so the
  cache is cleared and precached lines are re-rendered in the background.
- `Precache` fills it ahead of time. Do this for all menu texts at startup.
- Capped at ~64 MB (~25 min of speech). When full it empties and refills on demand.

Short version: the **queue** decides *when*, the **cache** makes it *fast*.

---

## 7. Engines (backends)

| Engine | Status | Notes |
|---|---|---|
| **Piper** | Default | Shipped with the game. Offline, neural, sounds natural. |
| **Windows OneCore** (Bengt) | Works | Player must install Swedish speech in Windows. Fallback if Piper is missing. |
| **SAPI5** (Acapela, Vocalizer ...) | Planned | Plan in `Sapi5SpeechBackend.h`. |
| Player's own API / screen reader | Idea | See section 10. |

All engines return a finished audio clip, so volume, timing and the events work the
same no matter which engine is used.

---

## 8. Piper in detail

[Piper](https://github.com/rhasspy/piper) is an offline neural TTS. We use release
`2023.11.14-2` and the voice `sv_SE-nst-medium` (22 050 Hz mono, dataset CC0, by KBLab).

### Why a separate process?

`piper.exe` runs as a **hidden child process** instead of being built into the game:

- **License:** Piper uses eSpeak NG (GPL) to turn text into phonemes. Keeping it in its
  own process keeps GPL code out of our game binary.
- **No DLL clash:** Piper uses ONNX Runtime, and Unreal ships its own version.
- **Simple:** no porting of Piper's code into Unreal.

The player never sees it. It only runs while Piper is the active engine, and it closes
when the game exits or another engine is picked. If the game crashes, Piper exits by
itself (its input closes).

### How one line is rendered (`FPiperSpeechBackend`)

```
worker thread                                piper.exe
─────────────                                ─────────
start once:  piper.exe --model <voice>.onnx --json-input
             --output_dir . --length_scale <1/speed>
             (working dir = Saved/Speech/Piper)

write stdin: {"text":"Hej!","output_file":"line_7.wav"}\n  ──>  renders
                                                           <──  prints "line_7.wav"
read Saved/Speech/Piper/line_7.wav -> raw audio -> delete file -> FSpeechClip
```

- Text is sent as UTF-8 JSON, so å/ä/ö, quotes and newlines are safe.
- Piper's log shares the same output pipe. Lines with `[error]` are logged as warnings,
  the rest as Verbose.
- **Voice and speed are fixed per process.** Changing them restarts piper.exe (~1 s,
  in the background). Speed maps to Piper's `length_scale = 1 / rate`.
- If Piper hangs (30 s timeout) or dies, the line is skipped and Piper restarts on
  the next line.
- Leftover WAV files from a crash are deleted at startup.

### Adding more Piper voices

Put `<name>.onnx` + `<name>.onnx.json` in `ThirdParty/Piper/voices/`
(voices: https://huggingface.co/rhasspy/piper-voices). They show up automatically
in `GetVoices()`. Name format `sv_SE-<speaker>-<quality>` gives language `sv-SE` and
display name `Piper <speaker>`. Also add the download to `Setup-Piper.ps1`.

---

## 9. Packaging

`AudioGameSpeech.Build.cs` adds the Piper files as `RuntimeDependencies`, so packaged
builds include them automatically (`*.exe`, `*.dll`, `espeak-ng-data/`, `voices/*.onnx`,
`voices/*.onnx.json`). Adds ~80 MB to the game.

**The build machine must have run `Setup-Piper.ps1`**, otherwise the package has no Piper
and falls back to Windows voices. Test a packaged build now and then.

---

## 10. Adding a new engine

1. Create `FMySpeechBackend : public ISpeechBackend` in `Private/`.
2. Implement:
   - `GetVoices()`: game thread, must be cheap. Give voices unique `Id`s and set `Engine`.
   - `Synthesize()`: worker thread, blocking. Fill `FSpeechClip` with 16-bit PCM
     (`SpeechWav.h` can parse WAV for you).
   - Optional: `InitThread()` / `ShutdownThread()` (setup/teardown, runs on the worker
     thread), `Prepare()` (warm up for a voice/speed).
3. Add a value to the end of `ESpeechEngine`.
4. Register it in `USpeechSubsystem::Initialize`: `Backends.Add(ESpeechEngine::X, MakeShared<FMySpeechBackend>());`

Engine switching, queue, cache and playback then work without other changes.

**Cloud APIs:** never ship API keys in game files. Let the player enter their own,
handle network delay, and keep Piper as fallback when offline.

**Screen readers (NVDA/JAWS via e.g. Tolk):** these speak by themselves and return no
audio, so they don't fit `Synthesize()` directly. They'd need a "pass-through" mode
without duration info (cutscene timing must handle that).

---

## 11. Troubleshooting

| Problem | Fix |
|---|---|
| Log: `Piper not found. Run ... Setup-Piper.ps1` | Run the script, restart the editor. |
| Log: `Piper voice missing` | `voices/*.onnx` or `.onnx.json` missing. Re-run the script. |
| Log: `Piper stopped (exit code X)` | Look at the lines before it. Check the voice files aren't corrupt. |
| No sound but no errors | Check `Set Volume` and the Sound Class volume. Run `log LogAudioGameSpeech Verbose` in the console for details. |
| First line is slow after changing speed | Expected: Piper restarts (~1 s). |
| `<winrt/...>` headers not found when building | See the comment in `Build.cs`. |
| Uses Bengt instead of Piper | Piper files missing (see first row). |

---

## 12. Known limits and planned work

**Known limits**

- Piper can't read its files if the game is **installed in a folder with å/ä/ö** in the
  path (Piper uses narrow-character paths).
- Only one Swedish Piper voice exists (`nst`).
- Numbers and abbreviations ("3", "t.ex.") may be read oddly. Write them out in the
  string tables, or add a text cleanup step later.
- Windows only.

**Planned work (search the code for these markers)**

| Marker | What |
|---|---|
| `Implement third party voice support here later` | SAPI5 voices. Plan in `Sapi5SpeechBackend.h`. |
| `TODO next step: priority queue` | Priorities, rule for outdated lines. |
| — | Engine/voice choice in the settings menu (saved between sessions). |
| — | Cutscene controller built on `OnSpeechStarted` / `OnSpeechFinished`. |

---

## 13. Rules

- **All synthesized speech goes through `USpeechSubsystem`.** Never create a second
  voice path.
- **All text in string tables** (Swedish now, more languages later).
- Speech keeps playing while the game is paused (by design, for menus).
- Don't commit Piper binaries or voices. Update `Setup-Piper.ps1` instead.
