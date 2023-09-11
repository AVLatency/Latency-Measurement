#pragma once
#include "AudioEndpoint.h"
#include <Audioclient.h>
#include "AbstractOutput.h"

class WasapiOutput : public AbstractOutput
{
public:
	struct OutputTiming
	{
		float BufferSizeInMilliseconds;
		LARGE_INTEGER BeforeStartQpcTime;
		LARGE_INTEGER AfterStartQpcTime;
	};

	///<param name="firstChannelOnly">If true, audio will only be output to the first channel. Otherwise audio will be output to all channels</param>
	///<param name="audioSamples">These provided samples must remain in memory so long as WasapiOutput might be reading them. They will not be deleted by WasapiOutput.</param>
	WasapiOutput(AudioEndpoint* endpoint, bool loop, bool firstChannelOnly, float* audioSamples, int audioSamplesLength, WAVEFORMATEX* waveFormat);
	~WasapiOutput();

	void StartPlayback();
	void StopPlayback();

	OutputTiming OutTiming;

	static INT16 FloatToINT16(float sample);
	static INT32 FloatToPaddedINT24(float sample);
	static INT32 FloatToINT32(float sample);

private:
	const AudioEndpoint* endpoint;
	bool loop;
	bool firstChannelOnly;
	float* audioSamples;
	int audioSamplesLength;
	WAVEFORMATEX* waveFormat;
	int sampleIndex = 0;

	// This is filled in only when waveFormat == WAVE_FORMAT_DOLBY_AC3_SPDIF
	char* dolbyAC3Samples = nullptr;
	int dolbyAC3SamplesLength = 0;
	int dolbyAC3SampleIndex = 0;

	std::atomic<bool> stopRequested = false;

	WORD GetFormatID();
	HRESULT LoadData(UINT32 bufferFrameCount, BYTE* pData, DWORD* flags);
	bool FinishedPlayback(bool loopIfNeeded);
};
