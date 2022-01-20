#pragma once
#include <string>
#include <map>
#include <Audioclient.h>

class OutputOffsetProfile
{
public:
	struct OutputOffset
	{
		float value; // in milliseconds, positive value means that analog leads digital, negative value means digital leads analog.
		bool verified;
	};

	/// <summary>
	/// Currently assumes LPCM format.
	/// Video signal and channel descriptions are excluded because these do not normally affect output offset.
	/// </summary>
	static std::string FormatStr(int numChannels, int sampleRate, int bitDepth);
	static std::string FormatStr(WAVEFORMATEX* waveFormat);

	std::string Name;
	std::map<std::string, OutputOffset> OutputOffsets;
};

