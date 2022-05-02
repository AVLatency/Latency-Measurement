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
	None->isNoOffset = true;
	Profiles.push_back(None);
}

OutputOffsetProfile* HdmiOutputOffsetProfiles::CurrentProfile()
{
	return Profiles[SelectedProfileIndex];
}

OutputOffsetProfile::OutputOffset HdmiOutputOffsetProfiles::HDV_MB01_GetOffset(int numChannels, int sampleRate, int bitDepth)
{
	OutputOffsetProfile::OutputOffset result;

	float v44100 = -0.1197f;
	float v48000 = -0.1030f;
	float v96000 = -0.0100f;
	float v192000 = 0.0181f;

	if (sampleRate == 44100) { result.SetValue(v44100); }
	else if (sampleRate == 48000) { result.SetValue(v48000); }
	else if (sampleRate == 96000) { result.SetValue(v96000); }
	else if (sampleRate == 192000) { result.SetValue(v192000); }
	else if (sampleRate < 44100) { result.value = v44100; result.verified = false; }
	else if (sampleRate > 44100 && sampleRate < 48000) { result.value = (v44100 + v48000) / 2; result.verified = false; }
	else if (sampleRate > 48000 && sampleRate < 96000) { result.value = (v48000 + v96000) / 2; result.verified = false; }
	else if (sampleRate > 96000 && sampleRate < 192000) { result.value = (v96000 + v192000) / 2; result.verified = false; }
	else if (sampleRate > 192000) { result.value = v192000; result.verified = false; }
	else {
#ifdef _DEBUG
		throw "I wrote this logic wrong. This code should never trigger.";
#else
		result.value = v48000;
		result.verified = false;
#endif
	}

	return result;
}

OutputOffsetProfile::OutputOffset HdmiOutputOffsetProfiles::None_GetOffset(int numChannels, int sampleRate, int bitDepth)
{
	OutputOffsetProfile::OutputOffset result;
	return result;
}