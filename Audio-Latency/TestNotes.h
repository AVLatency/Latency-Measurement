#pragma once
#include <imgui.h>
class TestNotes
{
public:
	static TestNotes Notes;

	bool DaulOutRefDeviceUseOutputOffsetProfile = true;
	char DaulOutRefDevice[128] = "";
	char DAC[128] = "";
	int RecordingMethodIndex = 0;
	const char* RecordingMethodOptions[4] = { "", "Microphone", "Direct Connection", "Other" };
	char RecordingMethodOther[128] = "";
	const char* RecordingMethod()
	{
		return (RecordingMethodIndex == IM_ARRAYSIZE(RecordingMethodOptions) - 1 ? RecordingMethodOther : RecordingMethodOptions[RecordingMethodIndex]);
	}

	char DutModel[128] = "";
	char DutFirmwareVersion[128] = "";
	int DutOutputTypeIndex = 0;
	const char* DutOutputTypeOptions[4] = { "", "Speaker", "Headphone Output", "Other" };
	char DutOutputTypeOther[128] = "";
	const char* DutOutputType()
	{
		return (DutOutputTypeIndex == IM_ARRAYSIZE(DutOutputTypeOptions) - 1 ? DutOutputTypeOther : DutOutputTypeOptions[DutOutputTypeIndex]);
	}
	int DutPassthroughOutputTypeIndex = 0;
	const char* DutPassthroughOutputTypeOptions[6] = { "", "ARC", "eARC", "S/PDIF Optical", "S/PDIF Coaxial", "Other" };
	char DutPassthroughOutputTypeOther[128] = "";
	const char* DutPassthroughOutputType()
	{
		return (DutPassthroughOutputTypeIndex == IM_ARRAYSIZE(DutPassthroughOutputTypeOptions) - 1 ? DutPassthroughOutputTypeOther : DutPassthroughOutputTypeOptions[DutPassthroughOutputTypeIndex]);
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
