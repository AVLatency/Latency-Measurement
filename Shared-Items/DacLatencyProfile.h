#pragma once
#include <string>
#include <map>
#include <Audioclient.h>
#include "AudioEndpoint.h"
#include "AVLTexture.h"

/// <summary>
/// All DACs must have a very similar latency to with all audio formats!!!
/// This is because the DAC may be given any audio format by the digital audio transmitter (e.g. the TV).
/// This class is currently exclusively used in the HDMI-to-Digital-Audio-Latency project.
/// </summary>
class DacLatencyProfile
{
public:
	enum struct DacInputType { Unknown, ARC, eARC, SPDIF_Optical, SPDIF_Coax };

	std::string Name;
	std::string Description;
	AVLTexture Image;
	bool isNoLatency = false; // Flag to help things like the TestManager and GUI know if this is the "None" offset profile.
	DacInputType InputType;
	float Latency = 0; // in milliseconds
};

