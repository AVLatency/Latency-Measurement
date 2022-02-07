#pragma once

#include "TestConfiguration.h"
#include "SystemInfo.h"
#include <atomic>
#include <vector>
#include <audioclient.h>

class TestManager
{
public:
	std::atomic<bool> IsFinished = false;
	std::atomic<bool> StopRequested = false;
	std::atomic<int> TotalPasses = 1;
	std::atomic<int> TotalRecordingsPerPass = 1;
	std::atomic<int> PassCount = 0;
	std::atomic<int> RecordingCount = 0;

	std::vector<WAVEFORMATEX*> SupportedFormats;
	std::vector<WAVEFORMATEX*> FailedFormats;

	TestManager(TestConfiguration config, const SystemInfo& sysInfo);
	~TestManager();

	void StartTest();

private:
	TestConfiguration config;
	const SystemInfo& systemInfo;

	int PopulateSupportedFormats();

	void SetBitsPerSample(WAVEFORMATEX* wfx, WORD bitsPerSample);

	bool PerformRecording(WAVEFORMATEX* waveFormat);
};

