#pragma once
#include "OutputOffsetProfile.h"
#include <vector>

/// <summary>
/// Describes the offset between HDMI output and analog output of
/// HDMI audio extractors.
/// </summary>
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

