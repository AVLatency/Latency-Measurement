#include "DacLatencyProfile.h"
#include <format>

std::string DacLatencyProfile::FormatStr(int numChannels, int sampleRate, int bitDepth)
{
	return std::format("{}-{}-{}", numChannels, sampleRate, bitDepth);
}

std::string DacLatencyProfile::FormatStr(WAVEFORMATEX* waveFormat)
{
	return FormatStr(waveFormat->nChannels, waveFormat->nSamplesPerSec, waveFormat->wBitsPerSample);
}