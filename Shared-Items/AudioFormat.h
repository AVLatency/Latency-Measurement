#pragma once
#include <Audioclient.h>
#include <string>
#include "FileAudioFormats.h"

class AudioFormat
{
public:
	enum struct FormatType
	{
		WaveFormatEx,
		File
	};

	struct FileParameters
	{
	public:
		FileAudioFormats::FileId id;
		std::string fileName;
		float duration;
		DWORD samplesPerSec;
		WORD numChannels;
		WORD bitsPerSample;
		std::string encoding;
		std::string speakersDescription;
	};

	FormatType type;
	std::string FormatString;

	FileAudioFormats::FileId FileId;
	std::string FileName;
	float FileDuration;
	DWORD FileSamplesPerSec;
	WORD FileNumChannels;
	WORD FileBitsPerSample;
	std::string FileEncoding;
	std::string FileSpeakersDescription;

	AudioFormat(WAVEFORMATEX* waveFormat);
	AudioFormat(FileParameters& params);

	WAVEFORMATEX* GetWaveFormat() const;
	DWORD GetSamplesPerSec() const;
	WORD GetNumChannels() const;
	WORD GetBitsPerSample() const;

	static WORD GetFormatID(WAVEFORMATEX* waveFormat);
	static std::string GetCurrentWinAudioFormatString();
	static std::string GetWaveFormatString(WAVEFORMATEX* waveFormat, bool includeEncoding, bool includeChannelInfo);
	static std::string GetChannelInfoString(WAVEFORMATEX* waveFormat);
	static std::string GetChannelInfoString(AudioFormat* format);
	static std::string GetAudioDataEncodingString(WAVEFORMATEX* waveFormat);
	static std::string GetAudioDataEncodingString(AudioFormat* format);

private:
	WAVEFORMATEX* waveFormat;
};

