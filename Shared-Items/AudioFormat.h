#pragma once
#include <Audioclient.h>
#include <string>

class AudioFormat
{
public:
	WAVEFORMATEX* WaveFormat;
	std::string FormatString;

	static std::string GetChannelInfoString(WAVEFORMATEX* waveFormat);

	AudioFormat(WAVEFORMATEX* waveFormat);
};

