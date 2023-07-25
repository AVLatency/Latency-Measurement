#pragma once
#include <string>
#include <map>
#include <Audioclient.h>
#include "AudioEndpoint.h"
#include "AVLTexture.h"

class OutputOffsetProfile
{
public:
	enum struct OutputType
	{
		None = 0,
		Hdmi,
		Spdif,
		ENUM_LENGTH
	};

	struct OutputOffset
	{
		float value = 0; // in milliseconds, positive value means that analog leads digital, negative value means digital leads analog.
		bool verified = false;

		/// <summary>
		/// Sets verified to true and sets the value.
		/// </summary>
		void SetValue(float value);
	};

	static std::string OutputTypeName(OutputOffsetProfile::OutputType outputType);
	
	OutputOffsetProfile(OutputType type, std::string name, OutputOffset (*getOffsetFunc)(int numChannels, int sampleRate, int bitDepth), bool (*formatFilter)(WAVEFORMATEX*));

	OutputType OutType = OutputType::None;
	std::string Name;
	std::string Description;
	AVLTexture Image;
	bool isNoOffset = false; // Flag to help things like the TestManager and GUI know if this is the "None" offset profile.

	OutputOffset(*GetOffset)(int numChannels, int sampleRate, int bitDepth);
	/// <summary>
	/// Used if a device takes in a digital audio format but then outputs a different audio format. For example,
	/// a DAC that takes in 5 channel audio, but then outputs a digital audio stream that incorrectly
	/// has only 2 channel audio. An example is an HDMI Audio Extractor that is used as a S/PDIF generator.
	/// </summary>
	bool (*FormatFilter)(WAVEFORMATEX*);

	OutputOffset GetOffsetFromWaveFormat(WAVEFORMATEX* waveFormat);


	/// <summary>
	/// Auto-populated by OutputOffsetProfiles::InitializeProfiles()
	/// </summary>
	std::vector<std::string> HighlightedVerifiedOffsetsForDisplay;
	/// <summary>
	/// Auto-populated by OutputOffsetProfiles::InitializeProfiles()
	/// </summary>
	std::vector<std::string> VerifiedOffsetsForDisplay;
	/// <summary>
	/// Auto-populated by OutputOffsetProfiles::InitializeProfiles()
	/// </summary>
	std::vector<std::string> UnverifiedOffsetsForDisplay;
};

