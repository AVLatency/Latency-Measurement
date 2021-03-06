#pragma once
#include <Audioclient.h>
#include "AudioEndpoint.h"

class WasapiInput
{
public:
	const int recordedAudioNumChannels = 2; // Be careful when changing this: Some code assumes two channel input.
	// Simple double buffer system. Assumes you will quickly copy the buffer soon after it has flipped.
	bool recordingToBuffer1 = true;
	float* recordingBuffer1 = nullptr;
	float* recordingBuffer2 = nullptr;
	int recordingBufferLength = 0;

	bool recordingInProgress = false;

	/// <summary>
	/// Only when not looping will this be set
	/// </summary>
	bool RecordingFailed = false;

	WAVEFORMATEXTENSIBLE waveFormat{};

	WasapiInput(const AudioEndpoint& endpoint, bool loop, double bufferDurationInSeconds);
	~WasapiInput();
	void StartRecording();
	void StopRecording();
private:
	bool loop;
	double bufferDurationInSeconds;
	int recordedAudioIndex = 0;
	bool stopRequested = false;
	const AudioEndpoint& endpoint;

	UINT16 GetFormatID();
	HRESULT SetFormat(WAVEFORMATEX* wfex);
	HRESULT CopyData(BYTE* pData, UINT32 bufferFrameCount, BOOL* bDone);
	bool FinishedRecording(bool flipBuffersIfNeeded);
	void ThrowAwayRecording();
	float* CurrentBuffer();
};

