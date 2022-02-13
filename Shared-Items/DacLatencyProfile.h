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

	std::string Name;
	std::map<std::string, DacLatency> Latencies;

	/// <summary>
	/// Used if a device takes in a digital audio format but then outputs a different audio format. For example,
	/// a DAC that takes in 5 channel audio, but then outputs a digital audio stream that incorrectly
	/// has only 2 channel audio. An example is an HDMI Audio Extractor that is used as a S/PDIF generator.
	/// </summary>
	bool (*FormatFilter)(WAVEFORMATEX*) = AudioEndpoint::AllFormatsFilter;
};

