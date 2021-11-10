#pragma once
#include <Audioclient.h>
#include "RecordingConfiguration.h"
#include <string>

class WasapiInput
{
public:
	WasapiInput(const RecordingConfiguration& config);
	~WasapiInput();
	void StartRecording();

	std::string DeviceName = "";

	const int recordedAudioNumChannels = 2; // Be careful when changing this: Some code assumes two channel input.
	float* recordedAudio = NULL;
	int recordedAudioLength = 0;

	bool RecordingFailed = false;

	WAVEFORMATEXTENSIBLE waveFormat{};

private:
	int recordedAudioIndex = 0;
	double testWaveDurationInSeconds;

	HRESULT SetFormat(WAVEFORMATEX* wfex);
	HRESULT CopyData(BYTE* pData, UINT32 bufferFrameCount, BOOL* bDone);
	UINT16 GetFormatID();
	bool FinishedRecording();
	void ThrowAwayRecording();
};

