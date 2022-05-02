#pragma once
#include <string>
#include <map>
#include <Audioclient.h>
#include "AudioEndpoint.h"

class OutputOffsetProfile
{
public:
	struct OutputOffset
	{
		float value = 0; // in milliseconds, positive value means that analog leads digital, negative value means digital leads analog.
		bool verified = false;

		/// <summary>
		/// Sets verified to true and sets the value.
		/// </summary>
		void SetValue(float value);
	};
	
	OutputOffsetProfile(std::string name, OutputOffset (*getOffsetFunc)(int numChannels, int sampleRate, int bitDepth), bool (*formatFilter)(WAVEFORMATEX*));

	std::string Name;
	OutputOffset(*GetOffset)(int numChannels, int sampleRate, int bitDepth);
	/// <summary>
	/// Used if a device takes in a digital audio format but then outputs a different audio format. For example,
	/// a DAC that takes in 5 channel audio, but then outputs a digital audio stream that incorrectly
	/// has only 2 channel audio. An example is an HDMI Audio Extractor that is used as a S/PDIF generator.
	/// </summary>
	bool (*FormatFilter)(WAVEFORMATEX*);

	OutputOffset GetOffsetFromWaveFormat(WAVEFORMATEX* waveFormat);

	bool isNoOffset = false; // Flag to help things like the TestManager know if this is the "None" offset profile.
};

