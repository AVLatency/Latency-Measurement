#pragma once
#include "WasapiInput.h"
#include "WasapiOutput.h"
#include <thread>
#include "AudioEndpoint.h"

class AdjustVolumeManager
{
public:
	WasapiOutput* output = nullptr;
	WasapiInput* input = nullptr;

	bool working = false;

	float* lastRecordedSegment = nullptr;
	int lastRecordedSegmentLength = 0;

	AdjustVolumeManager(const AudioEndpoint& outputEndpoint, const AudioEndpoint& inputEndpoint);
	~AdjustVolumeManager();
	void Tick();
	void Stop();

private:
	const double recordBufferDurationInSeconds = 0.5;
	bool lastBufferFlipWasTo1 = true;
	std::thread* outputThread = nullptr;
	std::thread* inputThread = nullptr;

	float* lastInputBufferCopy = nullptr;

	void CopyBuffer(float* sourceBuffer, int sourceBufferLength);
};

