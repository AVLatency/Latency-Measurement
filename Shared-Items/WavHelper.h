#pragma once
#include <string>

class WavHelper
{
public:
	/// <summary>
	/// firstChannelOnly will be ignored if numInputChannels > 1
	/// </summary>
	static void SaveWavFile(std::string path, std::string filename, float* samples, int samplesLength, int samplesPerSecond, int numInputChannels, int numOutputChannels, bool firstChannelOnly, float volume, int loopCount = 1);
};
