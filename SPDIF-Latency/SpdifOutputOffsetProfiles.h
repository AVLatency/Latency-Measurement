#pragma once
#include "OutputOffsetProfile.h"
#include <vector>

class SpdifOutputOffsetProfiles
{
public:
	static OutputOffsetProfile HDV_MB01;
	static OutputOffsetProfile AYSA11;
	static OutputOffsetProfile LiNKFOR_USB_DAC;
	static OutputOffsetProfile None;

	static std::vector<OutputOffsetProfile*> Profiles;
	static int SelectedProfileIndex;

	static void InitializeProfiles();

	static OutputOffsetProfile* CurrentProfile();

private:
	static bool CommonSpdifFormatFilter(WAVEFORMATEX*);
};

