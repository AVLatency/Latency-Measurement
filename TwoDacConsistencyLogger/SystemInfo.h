#pragma once
struct SystemInfo
{
public:
	char ComputerName[128] = "";
	char Display[128] = "N/A";
	char AudioOutputDrivers[128] = "";
	char VideoOutputDrivers[128] = "";
	char WindowsVersion[128] = "";
	char AdditionalNotes[512] = "";
	char Extractor[128] = "Extractor";
	char DAC[128] = "DAC";
	char VideoRes[128] = "1920x1080";
	char VideoRefresh[128] = "60.000";
	char VideoBitDepth[128] = "8";
	char VideoFormat[128] = "RGB";
	char VideoRange[128] = "SDR";
};

