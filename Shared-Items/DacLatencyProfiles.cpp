#include "DacLatencyProfiles.h"

DacLatencyProfile DacLatencyProfiles::None;
std::vector<DacLatencyProfile*> DacLatencyProfiles::Profiles;
int DacLatencyProfiles::SelectedProfileIndex = 0;

void DacLatencyProfiles::InitializeProfiles()
{
	None.Name = "Other";
	None.Latency = 0;
	Profiles.push_back(&None);
}

DacLatencyProfile* DacLatencyProfiles::CurrentProfile()
{
	return Profiles[SelectedProfileIndex];
}