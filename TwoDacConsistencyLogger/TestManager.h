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

	std::vector<WAVEFORMATEX*> AllHDMIFormats;
	std::vector<WAVEFORMATEX*> SupportedFormats;
	std::vector<WAVEFORMATEX*> FailedFormats;

	TestManager(TestConfiguration config, const SystemInfo& sysInfo);
	~TestManager();

	void StartTest();

private:
	TestConfiguration config;
	const SystemInfo& systemInfo;

	int PopulateSupportedFormats();
	void PopulateExFormats();
	void RecordExFormatTag(WAVEFORMATEX* exFormat);
	void RecordExChannels(WAVEFORMATEX* exFormat);
	void RecordExSamplesPerSec(WAVEFORMATEX* exFormat);
	void RecordExBitsPerSample(WAVEFORMATEX* exFormat);
	void RecordExFormat(WAVEFORMATEX* exFormat);

	void PopulateExtensibleFormats();
	void RecordExtensibleSubFormat(WAVEFORMATEXTENSIBLE* extensibleFormat);
	void RecordExtensibleChannels(WAVEFORMATEXTENSIBLE* extensibleFormat);
	void RecordExtensibleChannelMask(WAVEFORMATEXTENSIBLE* extensibleFormat);
	void RecordExtensibleSamplesPerSec(WAVEFORMATEXTENSIBLE* extensibleFormat);
	void RecordExtensibleBitsPerSample(WAVEFORMATEXTENSIBLE* extensibleFormat);
	void RecordExtensibleFormat(WAVEFORMATEXTENSIBLE* extensibleFormat);

	void SetBitsPerSample(WAVEFORMATEX* wfx, WORD bitsPerSample);

	bool PerformRecording(WAVEFORMATEX* waveFormat);
};

