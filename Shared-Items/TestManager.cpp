#include "TestManager.h"
#include <thread>
#include <Mmdeviceapi.h>
#include "WasapiOutput.h"
#include "WasapiInput.h"
#include "GeneratedSamples.h"
#include "RecordingAnalyzer.h"

TestManager::TestManager(const AudioEndpoint& outputEndpoint, const AudioEndpoint& inputEndpoint, std::vector<AudioFormat*> selectedFormats)
	: outputEndpoint(outputEndpoint), inputEndpoint(inputEndpoint), SelectedFormats(selectedFormats)
{
	managerThread = new std::thread([this] { this->StartTest(); });
}

TestManager::~TestManager()
{
	if (managerThread != NULL)
	{
		CleanUp();
	}
}

void TestManager::CleanUp()
{
	managerThread->join();
	delete(managerThread);
	managerThread = NULL;
}

void TestManager::StartTest()
{
	SetThreadExecutionState(ES_DISPLAY_REQUIRED); // Prevent display from turning off while running this tool.

	TotalPasses = TestConfiguration::NumMeasurements > 0 ? TestConfiguration::NumMeasurements : 1;
	TotalRecordingsPerPass = SelectedFormats.size() > 0 ? SelectedFormats.size() : 1;

	for (int i = 0; i < TestConfiguration::NumMeasurements; i++)
	{
		PassCount = i;
		RecordingCount = 0;
		for (AudioFormat* audioFormat : SelectedFormats)
		{
			if (!StopRequested && std::find(FailedFormats.begin(), FailedFormats.end(), audioFormat) == FailedFormats.end())
			{
				bool valid = PerformRecording(audioFormat);
				if (!valid)
				{
					FailedFormats.push_back(audioFormat);
				}
				RecordingCount++;
			}
		}
		int temp = RecordingCount;
		if (temp != TotalRecordingsPerPass)
		{
			TotalRecordingsPerPass = temp;
		}

		// TODO: inject a dummie one when we're down to a single recording
	}

	//RecordingAnalyzer::UpdateSummary(systemInfo);

	SetThreadExecutionState(0); // Reset prevent display from turning off while running this tool.

	IsFinished = true;
}

bool TestManager::PerformRecording(AudioFormat* audioFormat)
{
	GeneratedSamples* generatedSamples = new GeneratedSamples(audioFormat->WaveFormat, GeneratedSamples::WaveType::LatencyMeasurement);

	bool validResult = false;
	for (int i = 0; i < TestConfiguration::AttemptsBeforeFail; i++)
	{
		WasapiOutput* output = new WasapiOutput(outputEndpoint, false, generatedSamples->samples, generatedSamples->samplesLength, audioFormat->WaveFormat);
		std::thread outputThread{ [output] { output->StartPlayback(); } };

		WasapiInput* input = new WasapiInput(inputEndpoint, false, generatedSamples->TestWaveDurationInSeconds());
		std::thread inputThread{ [input] { input->StartRecording(); } };

		outputThread.join();
		inputThread.join();

		RecordingResult result = RecordingAnalyzer::AnalyzeRecording(*generatedSamples, *output, *input, TestConfiguration::DetectionThresholdMultiplier);
		validResult = result.Channel1.ValidResult && result.Channel2.ValidResult;

		delete output;
		delete input;
		
		if (validResult)
		{
			break;
		}
	}

	delete generatedSamples;
	return validResult;
}
