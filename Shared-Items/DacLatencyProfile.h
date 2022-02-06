#pragma once
#include <string>
#include <map>
#include <Audioclient.h>
#include "AudioEndpoint.h"

class DacLatencyProfile
{
public:
	struct DacLatency
	{
		float value; // in milliseconds
		bool verified;
	};

	/// <summary>
	/// Currently assumes LPCM format.
	/// Video signal and speaker descriptions are excluded because these do not normally affect output offset.
	/// </summary>
	static std::string FormatStr(int numChannels, int sampleRate, int bitDepth);
	static std::string FormatStr(WAVEFORMATEX* waveFormat);

	std::string Name;
	std::map<std::string, DacLatency> Latencies;
	bool (*FormatFilter)(WAVEFORMATEX*) = AudioEndpoint::AllFormatsFilter;
};

