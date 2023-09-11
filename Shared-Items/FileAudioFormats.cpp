#include "FileAudioFormats.h"
#include "AudioFormat.h"

FileAudioFormats FileAudioFormats::Formats;

FileAudioFormats::FileAudioFormats()
{
	AudioFormat::FileParameters params;
	params.id = FileId::DD_AC3_6ch_48kHz_16bit_448kbps_ffmpeg_wav;
	params.fileName = "DD-AC3_6ch-48kHz-16bit-448kbps-ffmpeg.wav";
	params.duration = 2.016;
	params.samplesPerSec = 48000;
	params.numChannels = 6;
	params.bitsPerSample = 16;
	params.encoding = "DD AC3";
	params.speakersDescription = "";
	AllFileFormats.push_back(new AudioFormat(params));

	params.id = FileId::DD_AC3_6ch_48kHz_16bit_448kbps_ffmpeg_wav;
	params.fileName = "DTS_6ch-48kHz-16bit-1412kbps-ffmpeg.wav";
	params.duration = 2.008;
	params.samplesPerSec = 48000;
	params.numChannels = 6;
	params.bitsPerSample = 16;
	params.encoding = "DTS";
	params.speakersDescription = "";
	AllFileFormats.push_back(new AudioFormat(params));
}
