#include "OutputOffsetProfiles.h"

OutputOffsetProfile* OutputOffsetProfiles::Hdmi_HDV_MB01 = NULL;
OutputOffsetProfile* OutputOffsetProfiles::Hdmi_None = NULL;

OutputOffsetProfile* OutputOffsetProfiles::Spdif_HDV_MB01 = NULL;
OutputOffsetProfile* OutputOffsetProfiles::Spdif_AYSA11 = NULL;
OutputOffsetProfile* OutputOffsetProfiles::Spdif_LiNKFOR_USB_DAC = NULL;
OutputOffsetProfile* OutputOffsetProfiles::Spdif_None = NULL;

std::vector<OutputOffsetProfile*> OutputOffsetProfiles::Profiles;
int OutputOffsetProfiles::SelectedProfileIndex = 0;

void OutputOffsetProfiles::InitializeProfiles()
{
	Hdmi_HDV_MB01 = new OutputOffsetProfile(OutputOffsetProfile::OutputType::Hdmi, "HDV-MB01", Hdmi_HDV_MB01_GetOffset, AudioEndpoint::HdmiFormatsFilter);
	Profiles.push_back(Hdmi_HDV_MB01);

	Hdmi_None = new OutputOffsetProfile(OutputOffsetProfile::OutputType::Hdmi, "Other", Hdmi_None_GetOffset, AudioEndpoint::HdmiFormatsFilter);
	Hdmi_None->isNoOffset = true;
	Profiles.push_back(Hdmi_None);


	Spdif_HDV_MB01 = new OutputOffsetProfile(OutputOffsetProfile::OutputType::Spdif, "HDV-MB01", Spdif_HDV_MB01_GetOffset, CommonSpdifFormatFilter);
	Profiles.push_back(Spdif_HDV_MB01);

	Spdif_AYSA11 = new OutputOffsetProfile(OutputOffsetProfile::OutputType::Spdif, "AYSA11", Spdif_AYSA11_GetOffset, CommonSpdifFormatFilter);
	Profiles.push_back(Spdif_AYSA11);

	Spdif_LiNKFOR_USB_DAC = new OutputOffsetProfile(OutputOffsetProfile::OutputType::Spdif, "LiNKFOR USB DAC", Spdif_LiNKFOR_USB_DAC_GetOffset, CommonSpdifFormatFilter);
	Profiles.push_back(Spdif_LiNKFOR_USB_DAC);

	Spdif_None = new OutputOffsetProfile(OutputOffsetProfile::OutputType::Spdif, "Other", Spdif_None_GetOffset, AudioEndpoint::AllFormatsFilter);
	Spdif_None->isNoOffset = true;
	Profiles.push_back(Spdif_None);
}

OutputOffsetProfile* OutputOffsetProfiles::CurrentProfile()
{
	return Profiles[SelectedProfileIndex];
}

OutputOffsetProfile::OutputOffset OutputOffsetProfiles::Hdmi_HDV_MB01_GetOffset(int numChannels, int sampleRate, int bitDepth)
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

OutputOffsetProfile::OutputOffset OutputOffsetProfiles::Hdmi_None_GetOffset(int numChannels, int sampleRate, int bitDepth)
{
	OutputOffsetProfile::OutputOffset result;
	return result;
}

bool OutputOffsetProfiles::CommonSpdifFormatFilter(WAVEFORMATEX* waveFormat)
{
	bool result = true;

	if (waveFormat->nChannels != 2)
	{
		result = false;
	}

	if (waveFormat->wBitsPerSample != 16
		&& waveFormat->wBitsPerSample != 20 // This hasn't been tested because the wave formats list doesn't include 20 bit samples yet.
		&& waveFormat->wBitsPerSample != 24)
	{
		result = false;
	}

	return result;
}

OutputOffsetProfile::OutputOffset OutputOffsetProfiles::Spdif_HDV_MB01_GetOffset(int numChannels, int sampleRate, int bitDepth)
{
	OutputOffsetProfile::OutputOffset result;

	if (numChannels == 2 && sampleRate == 44100 && (bitDepth == 16 || bitDepth == 24)) { result.SetValue(-0.2f); }
	else if (numChannels == 2 && sampleRate == 48000 && (bitDepth == 16 || bitDepth == 24)) { result.SetValue(-0.2f); }
	else if (numChannels == 2 && sampleRate == 96000 && (bitDepth == 16 || bitDepth == 24)) { result.SetValue(-0.1f); }
	else if (numChannels == 2 && sampleRate == 192000 && (bitDepth == 16 || bitDepth == 24)) { result.SetValue(0); }
	else { result.value = -0.1f; result.verified = false; } // Default to median offset

	return result;
}

OutputOffsetProfile::OutputOffset OutputOffsetProfiles::Spdif_AYSA11_GetOffset(int numChannels, int sampleRate, int bitDepth)
{
	OutputOffsetProfile::OutputOffset result;

	if (numChannels == 2 && sampleRate == 44100 && (bitDepth == 16 || bitDepth == 24)) { result.SetValue(-0.1f); }
	else if (numChannels == 2 && sampleRate == 48000 && (bitDepth == 16 || bitDepth == 24)) { result.SetValue(-0.1f); }
	else if (numChannels == 2 && sampleRate == 96000 && (bitDepth == 16 || bitDepth == 24)) { result.SetValue(0); }
	else if (numChannels == 2 && sampleRate == 192000 && (bitDepth == 16 || bitDepth == 24)) { result.SetValue(0); }
	else { result.value = 0; result.verified = false; } // Default to median offset

	return result;
}

OutputOffsetProfile::OutputOffset OutputOffsetProfiles::Spdif_LiNKFOR_USB_DAC_GetOffset(int numChannels, int sampleRate, int bitDepth)
{
	OutputOffsetProfile::OutputOffset result;

	if (numChannels == 2 && sampleRate == 48000 && bitDepth == 16) { result.SetValue(0); }
	if (numChannels == 2 && sampleRate == 44100 && bitDepth == 16) { result.SetValue(0); }
	else { result.value = 0; result.verified = false; }

	return result;
}

OutputOffsetProfile::OutputOffset OutputOffsetProfiles::Spdif_None_GetOffset(int numChannels, int sampleRate, int bitDepth)
{
	OutputOffsetProfile::OutputOffset result;
	return result;
}