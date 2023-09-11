#pragma once
#include <Mmdeviceapi.h>
#include <string>
#include <vector>
#include "SupportedAudioFormat.h"

class AudioEndpoint
{
public:
	IMMDevice* Device;
	std::string Name;
	std::string ID;

	std::vector<SupportedAudioFormat*> SupportedFormats;
	/// <summary>
	/// These additional formats are effectively duplicates of the ones in SupportedFormats.
	/// For example, if SupportedFormats includes a format with channel mask, DuplicateSupportedFormats
	/// may include the same format, but without the channel mask.
	/// </summary>
	std::vector<SupportedAudioFormat*> DuplicateSupportedFormats;
	
	AudioEndpoint(IMMDevice* device, std::string name, std::string id);
	~AudioEndpoint();
	void ClearFormats();

	void PopulateSupportedFormats(bool includeDuplicateFormats, bool includeSurroundAsDefault, bool ensureOneFormat, bool selectDefaults, bool (*formatFilter)(AudioFormat*));
	void SetDefaultFormats(bool includeSurroundAsDefault, bool ensureOneFormat, bool selectDefaults);

	std::vector<SupportedAudioFormat*> GetWaveFormatExFormats(int numChannels, int samplesPerSec, int bitsPerSample);
	std::vector<SupportedAudioFormat*> GetWaveFormatExFormats(int numChannels, int samplesPerSec);
	std::vector<SupportedAudioFormat*> GetWaveFormatExFormats(int numChannels);

private:
	bool AlreadyInSupportedFormats(WORD formatId, int numChannels, int samplesPerSec, int bitsPerSample);
	static int GetFormatIdOrder(WAVEFORMATEX* waveFormat);
};
