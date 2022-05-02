#pragma once
#include <string>
#include <map>
#include <Audioclient.h>
#include "AudioEndpoint.h"

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
	float Latency = 0; // in milliseconds
	DacInputType InputType;

	bool isNoLatency = false; // Flag to help things like the TestManager know if this is the "None" latency profile.
};

