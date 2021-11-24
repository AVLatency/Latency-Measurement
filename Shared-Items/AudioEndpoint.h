#pragma once
#include <Mmdeviceapi.h>
#include <string>

class AudioEndpoint
{
public:
	IMMDevice* Device;
	std::string Name;
	std::string ID;
	
	AudioEndpoint(IMMDevice* device, std::string name, std::string id);
	AudioEndpoint(const AudioEndpoint& other);
	AudioEndpoint(const AudioEndpoint&& other);
	~AudioEndpoint();
};
