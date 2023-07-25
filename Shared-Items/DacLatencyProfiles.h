#pragma once
#include "DacLatencyProfile.h"
#include <vector>
#include "ProfileResources.h"

class DacLatencyProfiles
{
public:
	static DacLatencyProfile CV121AD_ARC;
	static DacLatencyProfile CV121AD_SPDIF_COAX;
	static DacLatencyProfile CV121AD_SPDIF_OPTICAL;
	static DacLatencyProfile SHARCV1_EARC;
	static DacLatencyProfile None;

	static std::vector<DacLatencyProfile*> Profiles;
	static int SelectedProfileIndex;

	static void InitializeProfiles(ProfileResources& resources);

	static DacLatencyProfile* CurrentProfile();
};

