#pragma once
#include <Audioclient.h>
#include "AudioEndpoint.h"
#include <atomic>

class WasapiInput
{
public:
	const int recordedAudioNumChannels = 2; // Be careful when changing this: Some code assumes two channel input.
	// Simple double buffer system. Assumes you will quickly copy the buffer soon after it has flipped.
	std::atomic<bool> recordingToBuffer1 = true;
	float* recordingBuffer1 = nullptr;
	float* recordingBuffer2 = nullptr;
	int recordingBufferLength = 0;

	std::atomic<bool> recordingInProgress = false;

	WAVEFORMATEXTENSIBLE waveFormat{};

	WasapiInput(const AudioEndpoint& endpoint, bool loop, double bufferDurationInSeconds);
	~WasapiInput();
	void StartRecording();
	void StopRecording();
private:
	/// <summary>
	/// Only when not looping will this be set.
	/// This is currently not used, but might be useful in the future?
	/// </summary>
	bool RecordingFailed = false;

	bool loop;
	double bufferDurationInSeconds;
	int recordedAudioIndex = 0;
	std::atomic<bool> stopRequested = false;
	const AudioEndpoint& endpoint;

	UINT16 GetFormatID();
	HRESULT SetFormat(WAVEFORMATEX* wfex);
	HRESULT CopyData(BYTE* pData, UINT32 bufferFrameCount, BOOL* bDone);
	bool FinishedRecording(bool flipBuffersIfNeeded);
	void ThrowAwayRecording();
	float* CurrentBuffer();
};

