#pragma once
#include <Audioclient.h>
#include <string>

class AudioFormat
{
public:
	WAVEFORMATEX* WaveFormat;
	std::string FormatString;
	bool UserSelected = false;
	bool DefaultSelection = false;

	static std::string GetCurrentWinAudioFormatString();
	static std::string GetFormatString(WAVEFORMATEX* waveFormat, bool includeEncoding, bool includeChannelInfo);
	static std::string GetChannelInfoString(WAVEFORMATEX* waveFormat);
	static std::string GetAudioDataEncodingString(WAVEFORMATEX* waveFormat);

	AudioFormat(WAVEFORMATEX* waveFormat);
};

