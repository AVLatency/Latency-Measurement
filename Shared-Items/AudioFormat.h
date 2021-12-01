#pragma once
#include <Audioclient.h>
#include <string>

class AudioFormat
{
public:
	WAVEFORMATEX* WaveFormat;
	std::string FormatString;
	bool UserSelected = false;

	static std::string GetChannelInfoString(WAVEFORMATEX* waveFormat);

	AudioFormat(WAVEFORMATEX* waveFormat);
};

