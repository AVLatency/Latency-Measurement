#pragma once
#include "OutputOffsetProfile.h"
#include <vector>

class HdmiOutputOffsetProfiles
{
public:
	static OutputOffsetProfile HDV_MB01;
	static OutputOffsetProfile None;

	static std::vector<OutputOffsetProfile*> Profiles;
	static int SelectedProfileIndex;

	static void InitializeProfiles();

	static OutputOffsetProfile* CurrentProfile();
};

