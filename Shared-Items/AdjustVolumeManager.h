#pragma once
#include "WasapiInput.h"
#include "WasapiOutput.h"
#include <thread>
#include "AudioEndpoint.h"
#include "GeneratedSamples.h"

class AdjustVolumeManager
{
public:
	enum struct PeakLevelGrade { Good, Quiet, Crosstalk };

	struct VolumeAnalysis
	{
	public:
		bool CableCrosstalkDetection = true;

		int SampleRate = 0;

		float* RawWaveSamples = nullptr;
		int RawWaveSamplesLength = 0;
		float RawWavePeak = 0;

		float* AllEdges = nullptr;
		int AllEdgesLength = 0;
		float MaxEdgeIndex = 0;
		float MaxEdgeMagnitude = 0;

		int RawTickViewStartIndex = 0;
		int RawTickViewLength = 0;
		int RawFullViewStartIndex = 0;
		int RawFullViewLength = 0;

		// Monitor arrays are low-fi versions of the above arrays that sample
		// the max value for each sample bin
		float* TickMonitorSamples = nullptr;
		int TickMonitorSamplesLength = 0;
		float* FullMonitorSamples = nullptr;
		int FullMonitorSamplesLength = 0;

		float AutoThreshold = 0.1;
		PeakLevelGrade Grade = PeakLevelGrade::Quiet;
	};

	WasapiOutput* output = nullptr;
	WasapiInput* input = nullptr;

	bool working = false;

	VolumeAnalysis LeftVolumeAnalysis;
	VolumeAnalysis RightVolumeAnalysis;

	int TargetTickMonitorSampleLength;
	int TargetFullMonitorSampleLength;
	int TickMonitorCycles = 67; // exact number chosen because it shows lots of detail at 100% DpiScale.
	const float& UserLeftThreshold;
	const float& UserRightThreshold;

	AdjustVolumeManager(const AudioEndpoint& outputEndpoint, const AudioEndpoint& inputEndpoint, int targetTickMonitorSampleLength, int targetFullMonitorSampleLength, const float& userLeftThreshold, const float& userRightThreshold);
	~AdjustVolumeManager();
	void Tick();
	void Stop();

private:
	const double recordBufferDurationInSeconds = 0.5;
	bool lastBufferFlipWasTo1 = true;
	std::thread* outputThread = nullptr;
	std::thread* inputThread = nullptr;
	GeneratedSamples* generatedSamples = nullptr;

	void SafeResetVolumeAnalysis(VolumeAnalysis& analysis);
	WAVEFORMATEX* GetWaveFormat(const AudioEndpoint& endpoint);
	void SetBitsPerSample(WAVEFORMATEX* wfx, WORD bitsPerSample);

	void CopyBuffer(float* sourceBuffer, int sourceBufferLength);
	/// <summary>
	/// Memory ownership of recordedSamples array is transferred away from caller!
	/// </summary>
	void AnalyseChannel(VolumeAnalysis& analysis, float* recordedSamples, int recordedSamplesLength);

	float* CreateLowFiSamples(float* allSamples, int sourceStartIndex, int sourceLength, int destinationLength);
	void SetGrades();
	void CheckCableCrosstalk(VolumeAnalysis& analysis, VolumeAnalysis& other, const float& threshold);
	void SetChannelGrade(VolumeAnalysis& analysis, const float& threshold);
};

