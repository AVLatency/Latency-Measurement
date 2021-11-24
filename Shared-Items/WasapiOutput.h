#pragma once
#include "AudioEndpoint.h"
#include <Audioclient.h>

class WasapiOutput
{
public:
	bool playbackInProgress = false;

	/// <param name="waveFormat">If NULL, GetMixFormat will be used to get the current/default wave format.</param>
	WasapiOutput(const AudioEndpoint& endpoint, bool loop, float* audioSamples, int audioSamplesLength, WAVEFORMATEX* waveFormat = NULL);
	~WasapiOutput();

	void StartPlayback();
	void StopPlayback();

private:
	const AudioEndpoint& endpoint;
	bool loop;
	float* audioSamples;
	int audioSamplesLength;
	WAVEFORMATEX* waveFormat;
	int sampleIndex = 0;

	WORD GetFormatID();
	HRESULT LoadData(UINT32 bufferFrameCount, BYTE* pData, DWORD* flags);
};

