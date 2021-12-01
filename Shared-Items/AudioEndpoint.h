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

	std::vector<AudioFormat> SupportedFormats;
	
	AudioEndpoint(IMMDevice* device, std::string name, std::string id);
	AudioEndpoint(const AudioEndpoint& other);
	AudioEndpoint(const AudioEndpoint&& other);
	~AudioEndpoint();

	void PopulateSupportedFormats();
	void SelectDefaultFormats();

private:
	bool SupportsFormat(int numChannels, int samplesPerSec, int bitsPerSample);
	bool SupportsFormat(int numChannels, int samplesPerSec);

	std::vector<AudioFormat*> GetFormats(int numChannels, int samplesPerSec, int bitsPerSample);
};
