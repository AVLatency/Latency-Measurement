#pragma once
#include "WasapiInput.h"
#include "WasapiOutput.h"
#include <thread>
#include "AudioEndpoint.h"
#include "GeneratedSamples.h"

class AdjustVolumeManager
{
public:
	enum struct PeakLevelGrade { Good, Loud, Quiet };

	WasapiOutput* output = nullptr;
	WasapiInput* input = nullptr;

	bool working = false;

	float* leftChannelTickMonitorSamples = nullptr;
	float* rightChannelTickMonitorSamples = nullptr;
	float* rightChannelNormalizedTickMonitorSamples = nullptr;
	float* leftChannelTickReferenceSamples = nullptr;
	float* rightChannelNormalizedTickReferenceSamples = nullptr;
	int tickMonitorSamplesLength = 0;

	PeakLevelGrade leftChannelGrade = PeakLevelGrade::Quiet;
	PeakLevelGrade rightChannelGrade = PeakLevelGrade::Quiet;

	AdjustVolumeManager(const AudioEndpoint& outputEndpoint, const AudioEndpoint& inputEndpoint);
	~AdjustVolumeManager();
	void Tick();
	void Stop();

private:
	const double recordBufferDurationInSeconds = 0.5;
	bool lastBufferFlipWasTo1 = true;
	std::thread* outputThread = nullptr;
	std::thread* inputThread = nullptr;
	GeneratedSamples* generatedSamples = nullptr;

	void SafeDeleteMonitorSamples();
	WAVEFORMATEX* GetWaveFormat(const AudioEndpoint& endpoint);
	void SetBitsPerSample(WAVEFORMATEX* wfx, WORD bitsPerSample);

	void CopyBuffer(float* sourceBuffer, int sourceBufferLength);

	AdjustVolumeManager::PeakLevelGrade GetGrade(float value);
};

