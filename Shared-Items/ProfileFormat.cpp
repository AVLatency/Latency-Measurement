#include "ProfileFormat.h"
#include <format>

std::string ProfileFormat::FormatStr(int numChannels, int sampleRate, int bitDepth)
{
	return std::format("{}-{}-{}", numChannels, sampleRate, bitDepth);
}

std::string ProfileFormat::FormatStr(WAVEFORMATEX* waveFormat)
{
	return FormatStr(waveFormat->nChannels, waveFormat->nSamplesPerSec, waveFormat->wBitsPerSample);
}