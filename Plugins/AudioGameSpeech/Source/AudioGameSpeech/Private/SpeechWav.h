#pragma once

#include "CoreMinimal.h"
#include "SpeechTypes.h"

// Minimal WAV reader for 16-bit PCM.
// Reusable by a SAPI5 backend later.
inline bool ParseWav(const TArray<uint8>& Wav, FSpeechClip& Out)
{
	const int64 Num = Wav.Num();
	const uint8* D = Wav.GetData();

	auto Read16 = [D](int64 At) { return (uint16)(D[At] | (D[At + 1] << 8)); };
	auto Read32 = [D](int64 At) { return (uint32)D[At] | ((uint32)D[At + 1] << 8) | ((uint32)D[At + 2] << 16) | ((uint32)D[At + 3] << 24); };

	if (Num < 12 || FMemory::Memcmp(D, "RIFF", 4) != 0 || FMemory::Memcmp(D + 8, "WAVE", 4) != 0)
	{
		return false;
	}

	bool bFormatOk = false;
	int64 Pos = 12;

	while (Pos + 8 <= Num)
	{
		const uint32 ChunkSize = Read32(Pos + 4);
		const int64 Body = Pos + 8;

		if (FMemory::Memcmp(D + Pos, "fmt ", 4) == 0 && ChunkSize >= 16 && Body + 16 <= Num)
		{
			const uint16 Format = Read16(Body);          // 1 = PCM, 0xFFFE = extensible
			Out.NumChannels = Read16(Body + 2);
			Out.SampleRate  = (int32)Read32(Body + 4);
			const uint16 Bits = Read16(Body + 14);
			bFormatOk = (Format == 1 || Format == 0xFFFE) && Bits == 16 && Out.NumChannels > 0 && Out.SampleRate > 0;
		}
		else if (FMemory::Memcmp(D + Pos, "data", 4) == 0 && bFormatOk)
		{
			const int64 Size = FMath::Min<int64>(ChunkSize, Num - Body);
			if (Size <= 0)
			{
				return false;
			}
			Out.PcmData = TArray<uint8>(D + Body, (int32)Size);
			const double BytesPerSecond = (double)Out.SampleRate * Out.NumChannels * 2.0;
			Out.Duration = (float)(Size / BytesPerSecond);
			return true;
		}

		Pos = Body + ChunkSize + (ChunkSize & 1); // chunks are word aligned
	}

	return false;
}
