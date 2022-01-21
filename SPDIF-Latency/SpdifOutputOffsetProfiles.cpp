#include "SpdifOutputOffsetProfiles.h"

int SpdifOutputOffsetProfiles::SelectedProfileIndex = 0;
OutputOffsetProfile SpdifOutputOffsetProfiles::HDV_MB01;
OutputOffsetProfile SpdifOutputOffsetProfiles::AYSA11;
OutputOffsetProfile SpdifOutputOffsetProfiles::LiNKFOR_USB_DAC;
OutputOffsetProfile SpdifOutputOffsetProfiles::None;
std::vector<OutputOffsetProfile*> SpdifOutputOffsetProfiles::Profiles;

void SpdifOutputOffsetProfiles::InitializeProfiles()
{
	HDV_MB01.Name = "HDV-MB01";
	HDV_MB01.FormatFilter = CommonSpdifFormatFilter;
	HDV_MB01.OutputOffsets[OutputOffsetProfile::FormatStr(2, 44100, 16)] = OutputOffsetProfile::OutputOffset(-0.2, true);
	HDV_MB01.OutputOffsets[OutputOffsetProfile::FormatStr(2, 44100, 24)] = OutputOffsetProfile::OutputOffset(-0.2, true);
	HDV_MB01.OutputOffsets[OutputOffsetProfile::FormatStr(2, 48000, 16)] = OutputOffsetProfile::OutputOffset(-0.2, true);
	HDV_MB01.OutputOffsets[OutputOffsetProfile::FormatStr(2, 48000, 24)] = OutputOffsetProfile::OutputOffset(-0.2, true);
	HDV_MB01.OutputOffsets[OutputOffsetProfile::FormatStr(2, 96000, 16)] = OutputOffsetProfile::OutputOffset(-0.1, true);
	HDV_MB01.OutputOffsets[OutputOffsetProfile::FormatStr(2, 96000, 24)] = OutputOffsetProfile::OutputOffset(-0.1, true);
	HDV_MB01.OutputOffsets[OutputOffsetProfile::FormatStr(2, 192000, 16)] = OutputOffsetProfile::OutputOffset(0, true);
	HDV_MB01.OutputOffsets[OutputOffsetProfile::FormatStr(2, 192000, 24)] = OutputOffsetProfile::OutputOffset(0, true);
	Profiles.push_back(&HDV_MB01);
	
	AYSA11.Name = "AYSA11";
	AYSA11.FormatFilter = CommonSpdifFormatFilter;
	AYSA11.OutputOffsets[OutputOffsetProfile::FormatStr(2, 44100, 16)] = OutputOffsetProfile::OutputOffset(-0.1, true);
	AYSA11.OutputOffsets[OutputOffsetProfile::FormatStr(2, 44100, 24)] = OutputOffsetProfile::OutputOffset(-0.1, true);
	AYSA11.OutputOffsets[OutputOffsetProfile::FormatStr(2, 48000, 16)] = OutputOffsetProfile::OutputOffset(-0.1, true);
	AYSA11.OutputOffsets[OutputOffsetProfile::FormatStr(2, 48000, 24)] = OutputOffsetProfile::OutputOffset(-0.1, true);
	AYSA11.OutputOffsets[OutputOffsetProfile::FormatStr(2, 96000, 16)] = OutputOffsetProfile::OutputOffset(0, true);
	AYSA11.OutputOffsets[OutputOffsetProfile::FormatStr(2, 96000, 24)] = OutputOffsetProfile::OutputOffset(0, true);
	AYSA11.OutputOffsets[OutputOffsetProfile::FormatStr(2, 192000, 16)] = OutputOffsetProfile::OutputOffset(0, true);
	AYSA11.OutputOffsets[OutputOffsetProfile::FormatStr(2, 192000, 24)] = OutputOffsetProfile::OutputOffset(0, true);
	Profiles.push_back(&AYSA11);

	LiNKFOR_USB_DAC.Name = "LiNKFOR USB DAC";
	LiNKFOR_USB_DAC.FormatFilter = CommonSpdifFormatFilter;
	LiNKFOR_USB_DAC.OutputOffsets[OutputOffsetProfile::FormatStr(2, 44100, 16)] = OutputOffsetProfile::OutputOffset(0, true);
	LiNKFOR_USB_DAC.OutputOffsets[OutputOffsetProfile::FormatStr(2, 48000, 16)] = OutputOffsetProfile::OutputOffset(0, true);
	Profiles.push_back(&LiNKFOR_USB_DAC);

	None.Name = "Other";
	Profiles.push_back(&None);
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