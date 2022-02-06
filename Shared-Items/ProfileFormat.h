#pragma once
#include <Audioclient.h>
#include "AudioEndpoint.h"

class ProfileFormat
{
public:
	/// <summary>
	/// Currently assumes LPCM format.
	/// Video signal and speaker descriptions are excluded because these do not normally affect output offset or latency.
	/// </summary>
	static std::string FormatStr(int numChannels, int sampleRate, int bitDepth);
	static std::string FormatStr(WAVEFORMATEX* waveFormat);
};

