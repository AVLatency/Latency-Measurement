#include "HdmiOutputOffsetProfiles.h"

int HdmiOutputOffsetProfiles::SelectedProfileIndex = 0;
OutputOffsetProfile HdmiOutputOffsetProfiles::HDV_MB01;
OutputOffsetProfile HdmiOutputOffsetProfiles::None;
std::vector<OutputOffsetProfile*> HdmiOutputOffsetProfiles::Profiles;

void HdmiOutputOffsetProfiles::InitializeProfiles()
{
	HDV_MB01.Name = "HDV-MB01";
	Profiles.push_back(&HDV_MB01);

	None.Name = "None";
	Profiles.push_back(&None);
}

OutputOffsetProfile* HdmiOutputOffsetProfiles::CurrentProfile()
{
	return Profiles[SelectedProfileIndex];
}