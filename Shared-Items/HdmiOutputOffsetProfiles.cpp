#include "HdmiOutputOffsetProfiles.h"

int HdmiOutputOffsetProfiles::SelectedProfileIndex = 0;
OutputOffsetProfile HdmiOutputOffsetProfiles::HDV_MB01;
OutputOffsetProfile HdmiOutputOffsetProfiles::None;
std::vector<OutputOffsetProfile*> HdmiOutputOffsetProfiles::Profiles;

void HdmiOutputOffsetProfiles::InitializeProfiles()
{
	HDV_MB01.Name = "HDV-MB01";
	// TODO, something like this: HDV_MB01.OutputOffsets[ProfileFormat::FormatStr(2, 48000, 16)] = OutputOffsetProfile::OutputOffset(0, false);
	Profiles.push_back(&HDV_MB01);

	None.Name = "Other";
	Profiles.push_back(&None);
}

OutputOffsetProfile* HdmiOutputOffsetProfiles::CurrentProfile()
{
	return Profiles[SelectedProfileIndex];
}