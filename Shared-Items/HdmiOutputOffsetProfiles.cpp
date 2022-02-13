#include "HdmiOutputOffsetProfiles.h"

OutputOffsetProfile* HdmiOutputOffsetProfiles::HDV_MB01 = NULL;
OutputOffsetProfile* HdmiOutputOffsetProfiles::None = NULL;

std::vector<OutputOffsetProfile*> HdmiOutputOffsetProfiles::Profiles;
int HdmiOutputOffsetProfiles::SelectedProfileIndex = 0;

void HdmiOutputOffsetProfiles::InitializeProfiles()
{
	HDV_MB01 = new OutputOffsetProfile("HDV-MB01", HDV_MB01_GetOffset, AudioEndpoint::HdmiFormatsFilter);
	Profiles.push_back(HDV_MB01);

	None = new OutputOffsetProfile("Other", None_GetOffset, AudioEndpoint::HdmiFormatsFilter);
	Profiles.push_back(None);
}

OutputOffsetProfile* HdmiOutputOffsetProfiles::CurrentProfile()
{
	return Profiles[SelectedProfileIndex];
}

OutputOffsetProfile::OutputOffset HdmiOutputOffsetProfiles::HDV_MB01_GetOffset(int numChannels, int sampleRate, int bitDepth)
{
	OutputOffsetProfile::OutputOffset result;
	return result;
}

OutputOffsetProfile::OutputOffset HdmiOutputOffsetProfiles::None_GetOffset(int numChannels, int sampleRate, int bitDepth)
{
	OutputOffsetProfile::OutputOffset result;
	return result;
}