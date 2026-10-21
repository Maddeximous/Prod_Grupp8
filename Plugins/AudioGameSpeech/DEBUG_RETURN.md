# Return diagnostic: Piper speech silent on teammate's PC

Reply to `DEBUG_HANDOFF.md`. Tested 2026-10-07 on the teammate's PC, branch `Joche`
(not `feature/speech-core`; `DebugDumpClip` is not on this branch and was not built).
`git` is not on PATH in the shell here.

## Summary
**Piper and the plugin are fine. This PC's Windows audio output drops our sounds,
also outside Unreal.** Only Windows' own `tada.wav` was heard. Every WAV we generated,
in any format, was silent. This is a machine-specific Windows audio problem, not a code bug.

## Machine
- Default output (multimedia + communications): **Headphones (High Definition Audio Device)**
- Device/engine format: 2 ch, 48000 Hz, 16-bit (plain stereo, no surround configured)
- Effects on that output: **Microsoft Audio Home Theater Effects** (FxProperties present)
- Other active outputs: Digital Audio (S/PDIF), DELL U3011 and DELL S2722DC (NVIDIA HDMI/DP)
- Also installed: NVIDIA Virtual Audio Device (Wave Extensible)
- No third-party audio software running (no Nahimic, Dolby, Sonic Studio, Krisp, Voicemeeter, etc.)

## Piper files (handoff step 2): OK
- `espeak-ng-data/phontab` exists
- `voices/sv_SE-nst-medium.onnx` = 63,104,526 bytes
- `piper.exe` SHA256 = `96F3DA38...EDFA5C1F` (matches)

## Piper outside Unreal (handoff step 3): output is correct
`"Hej, det här är ett test." | piper.exe -m voices/sv_SE-nst-medium.onnx -f test.wav`
- 22050 Hz, mono, 16-bit, 1.88 s, valid RIFF/fmt/data
- Peak 100% (3 samples clipped), RMS ~17% (about -15 dBFS), DC offset -49 (none)
- Waveform looks like normal speech (fundamental ~190 Hz, silence at the end)
- Note: Piper exits 0 and writes nothing if the `-f` output folder does not exist.

## Listening tests (Windows `System.Media.SoundPlayer` -> default output)
| Test | File | Heard |
|---|---|---|
| A | `C:\Windows\Media\tada.wav` (44.1 kHz stereo, RMS ~500) | **yes** |
| B | Piper speech, 22050 Hz mono | no |
| C | Piper speech resampled to 48 kHz stereo (L = R) | no |
| D | 1 kHz sine, 22050 Hz mono | no |
| E | 150 Hz sine, 22050 Hz mono | no |
| G | Byte copy of tada.wav in a temp folder | **yes** |
| H | 1 kHz sine, 44.1 kHz stereo, quiet (amp 700, L = R) | no |
| I | 1 kHz sine, 44.1 kHz stereo, loud (amp 10000, L = R) | no |
| J/K/L | 800 Hz sine left only / right only / L and R out of phase | **not reported yet** |

What this rules out:
- Sample rate / channel count: H/I use tada's exact format and are still silent.
- Loudness: quiet and loud both silent.
- File location / file permissions: the tada copy in temp plays.
- Unreal: the same silence happens with plain Windows playback.

The only difference left is the **content**. Tada has real stereo differences and transients.
Everything we made is mono or has identical L/R (center-only), and steady tones or speech.

## Main suspect
**Microsoft Audio Home Theater Effects on the Headphones output** (or another enhancement
in that chain) removes center/identical-L-R content, like a "voice removal" effect. Unreal's
2D mono speech is center-panned, so it disappears too. Game sounds that are heard are probably
stereo or spatialized.

Not confirmed. The J/K/L result decides it:
- J/K/L audible -> center-removal effect confirmed.
- Still silent -> look further at the effects chain / driver.

## Next steps on this PC
1. Report J/K/L (left only / right only / out of phase).
2. Settings -> System -> Sound -> Headphones -> **Audio enhancements: Off**
   (or Control Panel -> Sound -> Headphones -> Properties -> Enhancements ->
   "Disable all enhancements"). Also check Spatial sound = Off.
3. Rerun test B (`test.wav`), then BP_SpeechTest in the editor. If it is audible now, done.
4. If not: update or reinstall the Realtek/HD Audio driver, or test another output (S/PDIF, monitor).

## Possible code change (only if we want to be robust against this)
No code bug was found. If players may have the same Windows setting, `PlayClip()` could output
stereo with a slight L/R difference (e.g. a few samples delay or a small pan) instead of
pure mono, so center-removal effects don't cancel it. Decide after step 2 confirms the cause.
