#pragma once
#include "AudioEndpoint.h"
#include <vector>
#include <mmdeviceapi.h>

class AudioEndpointHelper
{
public:
	static std::vector<AudioEndpoint> GetAudioEndPoints(EDataFlow type);

	static int GetInputMixFormatSampleRate(const AudioEndpoint& endpoint);
};

