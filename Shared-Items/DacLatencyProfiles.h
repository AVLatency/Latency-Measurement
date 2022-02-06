#pragma once
#include "DacLatencyProfile.h"
#include <vector>

class DacLatencyProfiles
{
public:
	static DacLatencyProfile None;

	static std::vector<DacLatencyProfile*> Profiles;
	static int SelectedProfileIndex;

	static void InitializeProfiles();

	static DacLatencyProfile* CurrentProfile();
};

