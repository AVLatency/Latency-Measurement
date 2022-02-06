#pragma once
#include <string>
#include <map>
#include <Audioclient.h>
#include "AudioEndpoint.h"

class OutputOffsetProfile
{
public:
	struct OutputOffset
	{
		float value; // in milliseconds, positive value means that analog leads digital, negative value means digital leads analog.
		bool verified;
	};

	std::string Name;
	std::map<std::string, OutputOffset> OutputOffsets;
	bool (*FormatFilter)(WAVEFORMATEX*) = AudioEndpoint::AllFormatsFilter;
};

