#pragma once
#include <Mmdeviceapi.h>
#include <string>
#include <vector>

class AudioFormat;

class FileAudioFormats
{
public:
	enum struct FileId
	{
		DD_AC3_6ch_48kHz_16bit_448kbps_ffmpeg_wav,
		DTS_6ch_48kHz_16bit_1412kbps_ffmpeg_wav
	};

	static FileAudioFormats Formats;

	std::vector<AudioFormat*> AllFileFormats;

	FileAudioFormats();
};
