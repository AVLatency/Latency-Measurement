#pragma once
#include "AudioEndpoint.h"
#include <Audioclient.h>

class WasapiOutput
{
public:
	bool playbackInProgress = false;

	///<param name="audioSamples">Memory for these samples will be owned by this object. They will be deleted in the deconstructor.</param>
	WasapiOutput(const AudioEndpoint& endpoint, bool loop, float* audioSamples, int audioSamplesLength, WAVEFORMATEX* waveFormat);
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

	bool stopRequested = false;

	WORD GetFormatID();
	HRESULT LoadData(UINT32 bufferFrameCount, BYTE* pData, DWORD* flags);
	bool FinishedPlayback(bool loopIfNeeded);
};

