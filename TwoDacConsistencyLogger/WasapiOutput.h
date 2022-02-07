#pragma once
#include <Audioclient.h>
#include <string>
#include "GeneratedSamples.h"

class WasapiOutput
{
public:
	WasapiOutput(const GeneratedSamples& config);
	void StartOutput();

	std::string DeviceName = "";

private:
	const GeneratedSamples& recordingConfig;
	WAVEFORMATEX* waveFormat;
	int sampleIndex = 0;

	WORD GetFormatID();
	HRESULT LoadData(UINT32 bufferFrameCount, BYTE* pData, DWORD* flags);
};

