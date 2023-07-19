#include "OutputOffsetProfile.h"

void OutputOffsetProfile::OutputOffset::SetValue(float value)
{
	this->value = value;
	verified = true;
}

OutputOffsetProfile::OutputOffsetProfile(OutputType type, std::string name, OutputOffset(*getOffsetFunc)(int numChannels, int sampleRate, int bitDepth), bool (*formatFilter)(WAVEFORMATEX*))
	: OutType(type), Name(name), GetOffset(getOffsetFunc), FormatFilter(formatFilter)
{
}

OutputOffsetProfile::OutputOffset OutputOffsetProfile::GetOffsetFromWaveFormat(WAVEFORMATEX* waveFormat)
{
	return GetOffset(waveFormat->nChannels, waveFormat->nSamplesPerSec, waveFormat->wBitsPerSample);
}