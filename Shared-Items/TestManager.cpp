#include "TestManager.h"
#include <thread>
#include <Mmdeviceapi.h>
#include "WasapiOutput.h"
#include "WasapiInput.h"
#include "GeneratedSamples.h"
#include "RecordingAnalyzer.h"
#include "StringHelper.h"

TestManager::TestManager(AudioEndpoint& outputEndpoint, const AudioEndpoint& inputEndpoint, std::vector<AudioFormat*> selectedFormats, std::string fileString, IResultsWriter& resultsWriter)
	: outputEndpoint(outputEndpoint), inputEndpoint(inputEndpoint), SelectedFormats(selectedFormats), GUID(StringHelper::GetGuidString()), resultsWriter(resultsWriter), Time(time(0))
{
	TestFileString = std::format("{} {} {}", StringHelper::GetTimeString(Time, true), fileString, GUID);

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
		AudioFormat* lastPlayedFormat = nullptr;
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
				lastPlayedFormat = audioFormat;
			}
		}
		int temp = RecordingCount;
		if (temp != TotalRecordingsPerPass)
		{
			TotalRecordingsPerPass = temp;
		}

		// Inject a dummy format when we're down to a single recording to force the HDMI Audio Device to re-sync to a new signal format
		if (!StopRequested && TotalRecordingsPerPass == 1)
		{
			bool formatSwitchResult = PlayFormatSwitch(lastPlayedFormat);
			if (!formatSwitchResult)
			{
				// something has gone very wrong, so bail early since there's no point in doing the same test over and over without switching formats.
				StopRequested = true; 
			}
		}
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

		RecordingResult result = RecordingAnalyzer::AnalyzeRecording(*generatedSamples, *input);
		
		if (TestConfiguration::SaveIndividualWavFiles)
		{
			std::string recordingFolder = format("{}/{}/{}", StringHelper::GetRootPath(), TestFileString, audioFormat->FormatString);
			RecordingAnalyzer::SaveRecording(*input, format("{}/{}.wav", recordingFolder, result.GUID));
		}
		if (TestConfiguration::SaveIndividualRecordingResults)
		{
			RecordingAnalyzer::SaveIndividualResult(resultsWriter, audioFormat, outputEndpoint, inputEndpoint, result, std::format("{}/{}", StringHelper::GetRootPath(), TestFileString));
		}

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

bool TestManager::PlayFormatSwitch(AudioFormat* lastPlayedFormat)
{
	WAVEFORMATEX* waveFormat = nullptr;

	// First, see if the stereo 48 kHz 16 bit is an option:
	{
		std::vector<AudioFormat*> formats = outputEndpoint.GetFormats(2, 48000, 16);
		waveFormat = FindFormatSwitchFormat(formats, lastPlayedFormat);
	}

	if (waveFormat == nullptr)
	{
		// Next, see if the stereo 44.1 kHz 16 bit is an option:
		std::vector<AudioFormat*> formats = outputEndpoint.GetFormats(2, 44100, 16);
		waveFormat = FindFormatSwitchFormat(formats, lastPlayedFormat);
	}

	if (waveFormat == nullptr)
	{
		// Next, see if the stereo 48 kHz is an option:
		std::vector<AudioFormat*> formats = outputEndpoint.GetFormats(2, 48000);
		waveFormat = FindFormatSwitchFormat(formats, lastPlayedFormat);
	}

	if (waveFormat == nullptr)
	{
		// Next, see if the stereo 44.1 kHz is an option:
		std::vector<AudioFormat*> formats = outputEndpoint.GetFormats(2, 44100);
		waveFormat = FindFormatSwitchFormat(formats, lastPlayedFormat);
	}

	if (waveFormat == nullptr)
	{
		// Next, see if the stereo is an option:
		std::vector<AudioFormat*> formats = outputEndpoint.GetFormats(2);
		waveFormat = FindFormatSwitchFormat(formats, lastPlayedFormat);
	}

	if (waveFormat == nullptr)
	{
		// This will probably only happen the measurements are coming back invalid due to a hardware configuraion issue.
		for (const AudioFormat& f : outputEndpoint.SupportedFormats)
		{
			if (lastPlayedFormat != &f && std::find(FailedFormats.begin(), FailedFormats.end(), &f) == FailedFormats.end())
			{
				waveFormat = f.WaveFormat;
				break;
			}
		}
	}

	if (waveFormat == nullptr)
	{
		// it seems like maybe this endpoint only actually outputs audio for a single format?
		return false;
	}

	GeneratedSamples* generatedSamples = new GeneratedSamples(waveFormat, GeneratedSamples::WaveType::LatencyMeasurement);
	WasapiOutput* output = new WasapiOutput(outputEndpoint, false, generatedSamples->samples, generatedSamples->samplesLength, waveFormat);
	std::thread outputThread{ [output] { output->StartPlayback(); } };
	outputThread.join();
	delete output;

	return true;
}

WAVEFORMATEX* TestManager::FindFormatSwitchFormat(std::vector<AudioFormat*> formats, AudioFormat* lastPlayedFormat)
{
	if (formats.size() > 0)
	{
		for (AudioFormat* f : formats)
		{
			if (lastPlayedFormat != f && std::find(FailedFormats.begin(), FailedFormats.end(), f) == FailedFormats.end())
			{
				return f->WaveFormat;
			}
		}
	}
	return nullptr;
}
