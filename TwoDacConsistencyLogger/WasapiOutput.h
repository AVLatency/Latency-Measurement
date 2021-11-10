#pragma once
#include <Audioclient.h>
#include <string>
#include "RecordingConfiguration.h"

class WasapiOutput
{
public:
	WasapiOutput(const RecordingConfiguration& config);
	void StartOutput();

	std::string DeviceName = "";

private:
	const RecordingConfiguration& recordingConfig;
	WAVEFORMATEX* waveFormat;
	int sampleIndex = 0;

	WORD GetFormatID();
	HRESULT LoadData(UINT32 bufferFrameCount, BYTE* pData, DWORD* flags);
};

