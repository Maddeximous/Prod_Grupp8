#pragma once

// =====================================================================
// Implement third party voice support here later.
// Not compiled or used yet. Placeholder so the plan is easy to find.
//
// Why: players' favorite Swedish voices (Acapela, Vocalizer etc.)
//      plug into SAPI5, which the OneCore backend can't see.
//
// Plan:
//  1. class FSapi5SpeechBackend : public ISpeechBackend
//  2. InitThread(): CoInitializeEx(nullptr, COINIT_MULTITHREADED)
//  3. GetVoices(): enumerate SPCAT_VOICES tokens.
//     Set Voice.Engine = ESpeechEngine::ThirdPartySAPI5
//  4. Synthesize(): speak into a memory stream (ISpStream, 16-bit PCM),
//     copy bytes into FSpeechClip. Duration = bytes / bytes-per-second.
//     Rate: SAPI5 uses -10..10, map from our 0.5..6.0.
//  5. USpeechSubsystem::Initialize: Backends.Add(ESpeechEngine::ThirdPartySAPI5, ...).
//     Engine switching (SetVoice) already works, nothing else to change.
// =====================================================================
