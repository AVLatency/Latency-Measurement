#include "DacLatencyProfiles.h"

int DacLatencyProfiles::SelectedProfileIndex = 0;
DacLatencyProfile DacLatencyProfiles::None;
std::vector<DacLatencyProfile*> DacLatencyProfiles::Profiles;

void DacLatencyProfiles::InitializeProfiles()
{
	None.Name = "Other";
	Profiles.push_back(&None);
}

DacLatencyProfile* DacLatencyProfiles::CurrentProfile()
{
	return Profiles[SelectedProfileIndex];
}