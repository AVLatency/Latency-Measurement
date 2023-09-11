#pragma once
#include "AudioEndpoint.h"
#include <vector>
#include <mmdeviceapi.h>

class AudioEndpointHelper
{
public:
	static std::vector<AudioEndpoint*> GetAudioEndPoints(EDataFlow type); // caller takes ownership of returned AudioEndpoint objects inside of the vector
	static AudioEndpoint* GetDefaultAudioEndPoint(EDataFlow type); // caller takes ownership of returned AudioEndpoint object

	static int GetInputMixFormatSampleRate(AudioEndpoint* endpoint);
};

