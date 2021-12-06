#pragma once

#include "TestConfiguration.h"
#include <vector>
#include <audioclient.h>
#include <thread>
#include "AudioFormat.h"
#include "AudioEndpoint.h"

class TestManager
{
public:
	bool IsFinished = false;
	bool StopRequested = false;
	int TotalPasses = 1;
	int TotalRecordingsPerPass = 1;
	int PassCount = 0;
	int RecordingCount = 0;

	std::vector<AudioFormat*> SelectedFormats;
	std::vector<AudioFormat*> FailedFormats;
	
	/// <summary>
	/// Will start the test on a new thread
	/// </summary>
	TestManager(const AudioEndpoint& outputEndpoint, const AudioEndpoint& inputEndpoint, std::vector<AudioFormat*> selectedFormats);
	~TestManager();
	/// <summary>
	/// Should be called from the originating thread when IsFinished == true. This will join and delete the test thread.
	/// </summary>
	void CleanUp();

private:
	std::thread* managerThread = NULL;
	const AudioEndpoint& outputEndpoint;
	const AudioEndpoint& inputEndpoint;

	void StartTest();
	bool PerformRecording(AudioFormat* audioFormat);
};

