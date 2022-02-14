#pragma once
#include <imgui.h>
class TestNotes
{
public:
	static TestNotes Notes;

	bool HDMIAudioDeviceUseOutputOffsetProfile = true;
	char HDMIAudioDevice[128] = "";
	bool DACUseLatencyProfile = true;
	char DAC[128] = "";

	char DutModel[128] = "";
	char DutFirmwareVersion[128] = "";
	int DutOutputTypeIndex = 0;
	const char* DutOutputTypeOptions[6] = { "", "ARC", "eARC", "S/PDIF Optical", "S/PDIF Coaxial", "Other"};
	char DutOutputTypeOther[128] = "";
	const char* DutOutputType()
	{
		return (DutOutputTypeIndex == IM_ARRAYSIZE(DutOutputTypeOptions) - 1 ? DutOutputTypeOther : DutOutputTypeOptions[DutOutputTypeIndex]);
	}
	char DutVideoMode[128] = "";
	char DutAudioSettings[128] = "";
	char DutOtherSettings[128] = "";

	int VideoResIndex = 0;
	const char* VideoResOptions[6] = { "", "1280x720", "1920x1080", "3840x2160", "7680x4320", "Other" };
	char VideoResolutionOther[128] = "";
	const char* VideoRes()
	{
		return (VideoResIndex == IM_ARRAYSIZE(VideoResOptions) - 1 ? VideoResolutionOther : VideoResOptions[VideoResIndex]);
	}
	char VideoRefreshRate[128] = "";
	char VideoBitDepth[128] = "";
	char VideoColorFormat[128] = "";
	int VideoColorSpaceIndex = 0;
	const char* VideoColorSpaceOptions[4] = { "", "SDR", "HDR", "Other" };
	char VideoColorSpaceOther[128] = "";
	const char* VideoColorSpace()
	{
		return (VideoColorSpaceIndex == IM_ARRAYSIZE(VideoColorSpaceOptions) - 1 ? VideoColorSpaceOther : VideoColorSpaceOptions[VideoColorSpaceIndex]);
	}

	char Notes1[128] = "";
	char Notes2[128] = "";
	char Notes3[128] = "";
	char Notes4[128] = "";
};
