#include "TestManager.h"
#include <thread>
#include <Mmdeviceapi.h>
#include "WasapiOutput.h"
#include "WasapiInput.h"
#include "RecordingConfiguration.h"
#include "RecordingAnalyzer.h"
#include "WindowsWaveFormats.h"

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

TestManager::TestManager(TestConfiguration config, const SystemInfo & sysInfo) : systemInfo{sysInfo}
{
	this->config = config;
}

TestManager::~TestManager()
{
	// todo: foreach waveformatex* in the formats do this? Or delte them?
	//CoTaskMemFree(pwfx);
}

void TestManager::StartTest()
{
	PopulateSupportedFormats();

	TotalPasses = config.NumberOfSingleRecordingPasses > 0 ? config.NumberOfSingleRecordingPasses : 1;
	TotalRecordingsPerPass = SupportedFormats.size() > 0 ? SupportedFormats.size() : 1;

	for (int i = 0; i < config.NumberOfSingleRecordingPasses; i++)
	{
		PassCount = i;
		RecordingCount = 0;
		for (WAVEFORMATEX* waveFormat : SupportedFormats)
		{
			if (!StopRequested && std::find(FailedFormats.begin(), FailedFormats.end(), waveFormat) == FailedFormats.end())
			{
				bool valid = PerformRecording(waveFormat);
				if (!valid)
				{
					FailedFormats.push_back(waveFormat);
				}
				RecordingCount++;
			}
		}
		int temp = RecordingCount;
		if (temp != TotalRecordingsPerPass)
		{
			TotalRecordingsPerPass = temp;
		}
	}
	for (WAVEFORMATEX* waveFormat : SupportedFormats)
	{
		for (int i = 0; i < config.NumberInBatchPass; i++)
		{
			if (!StopRequested && std::find(FailedFormats.begin(), FailedFormats.end(), waveFormat) == FailedFormats.end())
			{
				bool valid = PerformRecording(waveFormat);
				if (!valid)
				{
					FailedFormats.push_back(waveFormat);
				}
			}
		}
	}

	RecordingAnalyzer::UpdateSummary(systemInfo);

	IsFinished = true;
}

int TestManager::PopulateSupportedFormats()
{
	const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
	const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
	const IID IID_IAudioClient = __uuidof(IAudioClient);

	HRESULT hr;

	hr = CoInitialize(NULL);

	IMMDeviceEnumerator* pEnumerator = NULL;
	IMMDevice* pDevice = NULL;
	IAudioClient* pAudioClient = NULL;

	hr = CoCreateInstance(
		CLSID_MMDeviceEnumerator, NULL,
		CLSCTX_ALL, IID_IMMDeviceEnumerator,
		(void**)&pEnumerator);
	EXIT_ON_ERROR(hr)

		hr = pEnumerator->GetDefaultAudioEndpoint(
			eRender, eConsole, &pDevice);
	EXIT_ON_ERROR(hr)

		hr = pDevice->Activate(
			IID_IAudioClient, CLSCTX_ALL,
			NULL, (void**)&pAudioClient);
	EXIT_ON_ERROR(hr)

	for (WAVEFORMATEX* wfx : WindowsWaveFormats::Formats.AllExFormats)
	{
		HRESULT supportedHr = pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, wfx, NULL);
		if (supportedHr == S_OK)
		{
			// wfx is supported!
			SupportedFormats.push_back(wfx);
		}
	}

Exit:
	SAFE_RELEASE(pEnumerator)
	SAFE_RELEASE(pDevice)
	SAFE_RELEASE(pAudioClient)

	return SupportedFormats.size();
}

void TestManager::SetBitsPerSample(WAVEFORMATEX* wfx, WORD bitsPerSample)
{
	wfx->wBitsPerSample = bitsPerSample;
	wfx->nBlockAlign = wfx->wBitsPerSample / 8 * wfx->nChannels;
	wfx->nAvgBytesPerSec = wfx->nBlockAlign * wfx->nSamplesPerSec;
}

bool TestManager::PerformRecording(WAVEFORMATEX* waveFormat)
{
	RecordingConfiguration* recordingConfig = new RecordingConfiguration(waveFormat);

	bool validResult = false;
	for (int i = 0; i < config.NumberOfRetries + 1; i++)
	{
		WasapiOutput* output = new WasapiOutput(*recordingConfig);
		std::thread outputThread{ [output] { output->StartOutput(); } };

		WasapiInput* input = new WasapiInput(*recordingConfig);
		std::thread inputThread{ [input] { input->StartRecording(); } };

		outputThread.join();
		inputThread.join();

		RecordingResult result = RecordingAnalyzer::AnalyzeRecording(*recordingConfig, systemInfo, *output, *input, config.DetectionThresholdMultiplier);
		validResult = result.Channel1.ValidResult && result.Channel2.ValidResult;

		delete output;
		delete input;
		
		if (validResult)
		{
			break;
		}
	}

	delete recordingConfig;
	return validResult;
}
