#pragma once
#include <Mmdeviceapi.h>
#include <string>
#include <vector>
#include "AudioFormat.h"

class AudioEndpoint
{
public:
	IMMDevice* Device;
	std::string Name;
	std::string ID;

	std::vector<AudioFormat> SupportedFormats; // TODO: refs and pointers to these are used here and there, which isn't safe.
	/// <summary>
	/// These additional formats are effectively duplicates of the ones in SupportedFormats.
	/// For example, if SupportedFormats includes a format with channel mask, DuplicateSupportedFormats
	/// may include the same format, but without the channel mask.
	/// </summary>
	std::vector<AudioFormat> DuplicateSupportedFormats; // TODO: refs and pointers to these are used here and there, which isn't safe.
	
	AudioEndpoint(IMMDevice* device, std::string name, std::string id);
	AudioEndpoint(const AudioEndpoint& other);
	AudioEndpoint(const AudioEndpoint&& other);
	~AudioEndpoint();

	void PopulateSupportedFormats(bool includeDuplicateFormats, bool includeSurroundAsDefault, bool selectDefaults, bool (*formatFilter)(WAVEFORMATEX*));
	void SetDefaultFormats(bool includeSurroundAsDefault, bool selectDefaults);

	std::vector<AudioFormat*> GetFormats(int numChannels, int samplesPerSec, int bitsPerSample);
	std::vector<AudioFormat*> GetFormats(int numChannels, int samplesPerSec);
	std::vector<AudioFormat*> GetFormats(int numChannels);

	/// <summary>
	/// Does not filter any formats
	/// </summary>
	static bool AllFormatsFilter(WAVEFORMATEX* waveFormat);
	/// <summary>
	/// Filters out non-HDMI formats
	/// </summary>
	static bool HdmiFormatsFilter(WAVEFORMATEX* waveFormat);

private:
	bool SupportsFormat(int numChannels, int samplesPerSec, int bitsPerSample);
	bool SupportsFormat(int numChannels, int samplesPerSec);

};
