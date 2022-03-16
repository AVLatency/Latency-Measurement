#pragma once
#include "WasapiInput.h"
#include "WasapiOutput.h"
#include <thread>
#include "AudioEndpoint.h"
#include "GeneratedSamples.h"

class AdjustVolumeManager
{
public:
	enum struct PeakLevelGrade { Good, Loud, Quiet, Crosstalk };

	struct VolumeAnalysis
	{
	public:
		float PeakValue = 0;
		float* TickMonitorSamples = nullptr;
		int TickMonitorSamplesLength = 0;
		float* FullMonitorSamples = nullptr;
		int FullMonitorSamplesLength = 0;
		int TickPosition = 0;
		float MaxPlotValue = 1;
		float AutoThreshold = 0.1;
		PeakLevelGrade Grade = PeakLevelGrade::Quiet;
	};

	WasapiOutput* output = nullptr;
	WasapiInput* input = nullptr;

	bool working = false;

	VolumeAnalysis LeftVolumeAnalysis;
	VolumeAnalysis RightVolumeAnalysis;

	AdjustVolumeManager(const AudioEndpoint& outputEndpoint, const AudioEndpoint& inputEndpoint);
	~AdjustVolumeManager();
	void Tick();
	void Stop();

private:
	const double recordBufferDurationInSeconds = 1.5;
	bool lastBufferFlipWasTo1 = true;
	std::thread* outputThread = nullptr;
	std::thread* inputThread = nullptr;
	GeneratedSamples* generatedSamples = nullptr;

	void SafeResetVolumeAnalysis(VolumeAnalysis& analysis);
	WAVEFORMATEX* GetWaveFormat(const AudioEndpoint& endpoint);
	void SetBitsPerSample(WAVEFORMATEX* wfx, WORD bitsPerSample);

	void CopyBuffer(float* sourceBuffer, int sourceBufferLength);
	void AnalyseChannel(VolumeAnalysis& analysis, float* recordedSamples, int recordedSamplesLength);
};

