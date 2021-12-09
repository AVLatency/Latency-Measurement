#include "OutputOffsetProfile.h"
#include <format>

std::string OutputOffsetProfile::FormatStr(int numChannels, int sampleRate, int bitDepth)
{
	return std::format("{}-{}-{}", numChannels, sampleRate, bitDepth);
}

std::string OutputOffsetProfile::FormatStr(WAVEFORMATEX* waveFormat)
{
	return FormatStr(waveFormat->nChannels, waveFormat->nSamplesPerSec, waveFormat->wBitsPerSample);
}