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
		ARC,
		eARC,
		Analog,
		HdmiAudioPassthrough,
		RelativeWinAudio,
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
	static std::string OutputTypeNameFileSafe(OutputOffsetProfile::OutputType outputType);
	
	OutputOffsetProfile(OutputType type, std::string name, OutputOffset (*getLpcmOffsetFunc)(int numChannels, int sampleRate, int bitDepth), OutputOffset(*getFileOffsetFunc)(FileAudioFormats::FileId id), bool (*formatFilter)(AudioFormat*));

	OutputType OutType = OutputType::None;
	std::string Name;
	std::string Description;
	AVLTexture Image;
	bool isNoOffset = false; // Flag to help things like the TestManager and GUI know if this is the "None" offset profile.
	bool isCurrentWindowsAudioFormat = false; // If true, the AudioGraphOutput should be used instead of WasapiOutput

	OutputOffset(*GetLpcmOffset)(int numChannels, int sampleRate, int bitDepth);
	OutputOffset(*GetFileOffset)(FileAudioFormats::FileId id);

	/// <summary>
	/// Used if a device takes in a digital audio format but then outputs a different audio format. For example,
	/// a DAC that takes in 5 channel audio, but then outputs a digital audio stream that incorrectly
	/// has only 2 channel audio. An example is an HDMI Audio Extractor that is used as a S/PDIF generator.
	/// This is also used to filter out formats that do not have an output offset measured.
	/// </summary>
	bool (*FormatFilter)(AudioFormat*);

	OutputOffset GetOffsetForFormat(AudioFormat* format);
	OutputOffset GetOffsetForWaveFormat(WAVEFORMATEX* waveFormat);
	OutputOffset GetOffsetForCurrentWinFormat();


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

