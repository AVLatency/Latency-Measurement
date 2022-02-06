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
	bool (*FormatFilter)(WAVEFORMATEX*) = AudioEndpoint::AllFormatsFilter;
};

