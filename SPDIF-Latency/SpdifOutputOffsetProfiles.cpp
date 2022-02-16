#include "SpdifOutputOffsetProfiles.h"

OutputOffsetProfile* SpdifOutputOffsetProfiles::HDV_MB01;
OutputOffsetProfile* SpdifOutputOffsetProfiles::AYSA11;
OutputOffsetProfile* SpdifOutputOffsetProfiles::LiNKFOR_USB_DAC;
OutputOffsetProfile* SpdifOutputOffsetProfiles::None;

std::vector<OutputOffsetProfile*> SpdifOutputOffsetProfiles::Profiles;
int SpdifOutputOffsetProfiles::SelectedProfileIndex = 0;

void SpdifOutputOffsetProfiles::InitializeProfiles()
{
	HDV_MB01 = new OutputOffsetProfile("HDV-MB01", HDV_MB01_GetOffset, CommonSpdifFormatFilter);
	Profiles.push_back(HDV_MB01);

	AYSA11 = new OutputOffsetProfile("AYSA11", AYSA11_GetOffset, CommonSpdifFormatFilter);
	Profiles.push_back(AYSA11);

	LiNKFOR_USB_DAC = new OutputOffsetProfile("LiNKFOR USB DAC", LiNKFOR_USB_DAC_GetOffset, CommonSpdifFormatFilter);
	Profiles.push_back(LiNKFOR_USB_DAC);

	None = new OutputOffsetProfile("Other", None_GetOffset, AudioEndpoint::AllFormatsFilter);
	Profiles.push_back(None);
}

OutputOffsetProfile* SpdifOutputOffsetProfiles::CurrentProfile()
{
	return Profiles[SelectedProfileIndex];
}

bool SpdifOutputOffsetProfiles::CommonSpdifFormatFilter(WAVEFORMATEX* waveFormat)
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

OutputOffsetProfile::OutputOffset SpdifOutputOffsetProfiles::HDV_MB01_GetOffset(int numChannels, int sampleRate, int bitDepth)
{
	OutputOffsetProfile::OutputOffset result;

	if (numChannels == 2 && sampleRate == 44100 && (bitDepth == 16 || bitDepth == 24)) { result.SetValue(-0.2f); }
	else if (numChannels == 2 && sampleRate == 48000 && (bitDepth == 16 || bitDepth == 24)) { result.SetValue(-0.2f); }
	else if (numChannels == 2 && sampleRate == 96000 && (bitDepth == 16 || bitDepth == 24)) { result.SetValue(-0.1f); }
	else if (numChannels == 2 && sampleRate == 192000 && (bitDepth == 16 || bitDepth == 24)) { result.SetValue(0); }
	else { result.value = -0.1f; result.verified = false; } // Default to median offset

	return result;
}

OutputOffsetProfile::OutputOffset SpdifOutputOffsetProfiles::AYSA11_GetOffset(int numChannels, int sampleRate, int bitDepth)
{
	OutputOffsetProfile::OutputOffset result;

	if (numChannels == 2 && sampleRate == 44100 && (bitDepth == 16 || bitDepth == 24)) { result.SetValue(-0.1f); }
	else if (numChannels == 2 && sampleRate == 48000 && (bitDepth == 16 || bitDepth == 24)) { result.SetValue(-0.1f); }
	else if (numChannels == 2 && sampleRate == 96000 && (bitDepth == 16 || bitDepth == 24)) { result.SetValue(0); }
	else if (numChannels == 2 && sampleRate == 192000 && (bitDepth == 16 || bitDepth == 24)) { result.SetValue(0); }
	else { result.value = 0; result.verified = false; } // Default to median offset

	return result;
}

OutputOffsetProfile::OutputOffset SpdifOutputOffsetProfiles::LiNKFOR_USB_DAC_GetOffset(int numChannels, int sampleRate, int bitDepth)
{
	OutputOffsetProfile::OutputOffset result;

	if (numChannels == 2 && sampleRate == 48000 && bitDepth == 16) { result.SetValue(0); }
	if (numChannels == 2 && sampleRate == 44100 && bitDepth == 16) { result.SetValue(0); }
	else { result.value = 0; result.verified = false; }

	return result;
}

OutputOffsetProfile::OutputOffset SpdifOutputOffsetProfiles::None_GetOffset(int numChannels, int sampleRate, int bitDepth)
{
	OutputOffsetProfile::OutputOffset result;
	return result;
}