#pragma once
#include "OutputOffsetProfile.h"
#include <vector>

class SpdifOutputOffsetProfiles
{
public:
	static OutputOffsetProfile* HDV_MB01;
	static OutputOffsetProfile* AYSA11;
	static OutputOffsetProfile* LiNKFOR_USB_DAC;
	static OutputOffsetProfile* None;

	static std::vector<OutputOffsetProfile*> Profiles;
	static int SelectedProfileIndex;

	static void InitializeProfiles();

	static OutputOffsetProfile* CurrentProfile();

private:
	static bool CommonSpdifFormatFilter(WAVEFORMATEX*);

	static OutputOffsetProfile::OutputOffset HDV_MB01_GetOffset(int numChannels, int sampleRate, int bitDepth);
	static OutputOffsetProfile::OutputOffset AYSA11_GetOffset(int numChannels, int sampleRate, int bitDepth);
	static OutputOffsetProfile::OutputOffset LiNKFOR_USB_DAC_GetOffset(int numChannels, int sampleRate, int bitDepth);
	static OutputOffsetProfile::OutputOffset None_GetOffset(int numChannels, int sampleRate, int bitDepth);
};

