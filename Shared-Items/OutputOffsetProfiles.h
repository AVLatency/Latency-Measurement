#pragma once
#include "OutputOffsetProfile.h"
#include <vector>

/// <summary>
/// Describes the offset between digital output and analog output of
/// dual-out reference devices.
/// </summary>
class OutputOffsetProfiles
{
public:
	static OutputOffsetProfile* Hdmi_HDV_MB01;
	static OutputOffsetProfile* Hdmi_None;

	static std::vector<OutputOffsetProfile*> Profiles;
	static int SelectedProfileIndex;

	static void InitializeProfiles();

	static OutputOffsetProfile* CurrentProfile();

private:
	static OutputOffsetProfile::OutputOffset Hdmi_HDV_MB01_GetOffset(int numChannels, int sampleRate, int bitDepth);
	static OutputOffsetProfile::OutputOffset Hdmi_None_GetOffset(int numChannels, int sampleRate, int bitDepth);
};

