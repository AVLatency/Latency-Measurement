#include "TestManager.h"
#include <thread>
#include <Mmdeviceapi.h>
#include "WasapiOutput.h"
#include "WasapiInput.h"
#include "RecordingConfiguration.h"
#include "RecordingAnalyzer.h"

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
	PopulateExtensibleFormats();
	PopulateExFormats();
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

	for (WAVEFORMATEX* wfx : AllHDMIFormats)
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

void TestManager::PopulateExFormats()
{
	WAVEFORMATEX exFormat;
	memset(&exFormat, 0, sizeof(WAVEFORMATEX));
	exFormat.wFormatTag = WAVE_FORMAT_PCM;
	exFormat.nChannels = 1;
	exFormat.nSamplesPerSec = 44100;
	SetBitsPerSample(&exFormat, 16);
	exFormat.cbSize = 0;

	RecordExFormatTag(&exFormat);
}

void TestManager::RecordExFormatTag(WAVEFORMATEX* exFormat)
{
	exFormat->wFormatTag = WAVE_FORMAT_PCM;
	RecordExChannels(exFormat);

	exFormat->wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
	RecordExChannels(exFormat);
}

void TestManager::RecordExChannels(WAVEFORMATEX* exFormat)
{
	// For exclusive-mode formats, the method queries the device driver.
	// Some device drivers will report that they support a 1-channel or 2-channel
	// PCM format if the format is specified by a stand-alone WAVEFORMATEX structure, 
	// but will reject the same format if it is specified by a WAVEFORMATEXTENSIBLE structure.
	// To obtain reliable results from these drivers, exclusive-mode applications
	// should call IsFormatSupported twice for each 1-channel or 2-channel PCM format�one
	// call should use a stand-alone WAVEFORMATEX structure to specify the format, and the
	// other call should use a WAVEFORMATEXTENSIBLE structure to specify the same format.
	// Source: https://docs.microsoft.com/en-us/windows/win32/coreaudio/device-formats
	//
	// tl;dr: WAVEFORMATEX only needs to be populated with 1 or 2 channel PCM formats.
	// The rest of the formats can be handled by the WAVEFORMATEXTENSIBLE formats.

	exFormat->nChannels = 1;
	RecordExSamplesPerSec(exFormat);

	exFormat->nChannels = 2;
	RecordExSamplesPerSec(exFormat);
}

void TestManager::RecordExSamplesPerSec(WAVEFORMATEX* exFormat)
{
	exFormat->nSamplesPerSec = 44100;
	RecordExBitsPerSample(exFormat);

	exFormat->nSamplesPerSec = 48000;
	RecordExBitsPerSample(exFormat);

	exFormat->nSamplesPerSec = 96000;
	RecordExBitsPerSample(exFormat);

	exFormat->nSamplesPerSec = 192000;
	RecordExBitsPerSample(exFormat);
}

void TestManager::RecordExBitsPerSample(WAVEFORMATEX* exFormat)
{
	SetBitsPerSample(exFormat, 16);
	RecordExFormat(exFormat);

	SetBitsPerSample(exFormat, 24);
	RecordExFormat(exFormat);

	SetBitsPerSample(exFormat, 32);
	RecordExFormat(exFormat);
}

void TestManager::RecordExFormat(WAVEFORMATEX* exFormat)
{
	WAVEFORMATEX* pWfx = new WAVEFORMATEX();
	*pWfx = *exFormat;
	AllHDMIFormats.push_back(pWfx);
}

void TestManager::PopulateExtensibleFormats()
{
	WAVEFORMATEXTENSIBLE extensibleFormat;
	memset(&extensibleFormat, 0, sizeof(WAVEFORMATEXTENSIBLE));
	extensibleFormat.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	extensibleFormat.Format.nChannels = 1;
	extensibleFormat.Format.nSamplesPerSec = 44100;
	SetBitsPerSample(&extensibleFormat.Format, 16);
	extensibleFormat.Format.cbSize = 22;
	extensibleFormat.Samples.wValidBitsPerSample = (WORD)extensibleFormat.Format.wBitsPerSample;
	extensibleFormat.dwChannelMask = 0; // This could be all types of who knows!
	extensibleFormat.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;

	RecordExtensibleSubFormat(&extensibleFormat);
}

void TestManager::RecordExtensibleSubFormat(WAVEFORMATEXTENSIBLE* extensibleFormat)
{
	extensibleFormat->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
	RecordExtensibleChannels(extensibleFormat);

	extensibleFormat->SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
	RecordExtensibleChannels(extensibleFormat);
}

void TestManager::RecordExtensibleChannels(WAVEFORMATEXTENSIBLE* extensibleFormat)
{
	for (int i = 1; i < 9; i++)
	{
		extensibleFormat->Format.nChannels = (WORD)i;
		RecordExtensibleChannelMask(extensibleFormat);
	}
}

void TestManager::RecordExtensibleChannelMask(WAVEFORMATEXTENSIBLE* extensibleFormat)
{
	// This list is based off of the configurations that are possible with the Murideo SEVEN Generator
	// (8K SEVEN Generator User Manual page 38 "AUDIO CHANNEL /SPEAKER CONFIGURATION")
	// https://www.murideo.com/mu-gen-seven-g8k.html
	// https://www.murideo.com/seven-g-training.html
	// A few additional configurations have been added that are common Windows configurations, taken from ksmedia.h

	// Always start with a 0 channel mask to see if the IsFormatSupported function gives us a closest match
	// with actual speaker configurations.
	extensibleFormat->dwChannelMask = 0;
	RecordExtensibleSamplesPerSec(extensibleFormat);

	// This is just a guess on my part as to what speaker configurations might possible come up.
	// Who knows if this is at all necessary.
	switch (extensibleFormat->Format.nChannels)
	{
	case 1:
		// Windows Mono
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_CENTER; // KSAUDIO_SPEAKER_MONO;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		break;
	case 2:
		// 2.0
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT; // KSAUDIO_SPEAKER_STEREO
		RecordExtensibleSamplesPerSec(extensibleFormat);

		break;
	case 3:
		// 3.0 FC_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);
		// 3.0 RC_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		// 2.1 LFE_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_LOW_FREQUENCY;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		break;
	case 4:
		// 4.0 Windows Quadrophonic
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT; // KSAUDIO_SPEAKER_QUAD
		RecordExtensibleSamplesPerSec(extensibleFormat);
		// 4.0 RR_RL_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT;
		RecordExtensibleSamplesPerSec(extensibleFormat);
		// 4.0 RC_FC_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_BACK_CENTER; // KSAUDIO_SPEAKER_SURROUND
		RecordExtensibleSamplesPerSec(extensibleFormat);

		// 4.0 FRC_FLC_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		// 3.1 FC_LFE_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_LOW_FREQUENCY | SPEAKER_FRONT_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		// 3.1 RC_LFE_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		break;
	case 5:
		// 5.0 RR_RL_FC_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT;
		RecordExtensibleSamplesPerSec(extensibleFormat);
		// 5.0 RC_RR_RL_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_CENTER | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		// 5.0 FRC_FLC_FC_FR_LF
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);
		// 5.0 FRC_FLC_RC_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_CENTER | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		// 4.1 RC_FC_LFE_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_LOW_FREQUENCY | SPEAKER_FRONT_CENTER | SPEAKER_BACK_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);
		// 4.1 RR_RL_LFE_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_LOW_FREQUENCY | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		// 4.1 FRC_FLC_LFE_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_LOW_FREQUENCY | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		break;
	case 6:
		// 5.1 Windows DVD
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT; // KSAUDIO_SPEAKER_5POINT1
		RecordExtensibleSamplesPerSec(extensibleFormat);
		// 5.1 RR_RL_FC_LFE_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT; // KSAUDIO_SPEAKER_5POINT1_SURROUND
		RecordExtensibleSamplesPerSec(extensibleFormat);
		// 5.1 RC_RR_RL_LFE_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		// 5.1 FRC_FLC_FC_LFE_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);
		// 5.1 FRC_FLC_RC_FC_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_BACK_CENTER | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		// 6.0 RC_RR_RL_FC_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT | SPEAKER_BACK_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);
		// 6.0 RRC_RLC_RR_RL_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		// 6.0 FRC_FLC_RC_FC_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER | SPEAKER_BACK_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		// 6.0 FRC_FLC_RR_RL_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		break;
	case 7:
		// 7.0 RRC_RLC_RR_RL_FC_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		// 6.1 RC_RR_RL_FC_LFE_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT | SPEAKER_BACK_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);
		// 6.1 RRC_RLC_RR_RL_LFE_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_LOW_FREQUENCY  | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		// 6.1 FRC_FLC_RC_FC_LFE_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_LOW_FREQUENCY | SPEAKER_FRONT_CENTER | SPEAKER_BACK_CENTER | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);
		// 6.1 FRC_FLC_RR_RL_LFE_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_LOW_FREQUENCY | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		// 7.0 FRC_FLC_RR_RL_FC_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		break;
	case 8: 
		// 7.1 RRC_RLC_RR_RL_FC_LFE_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT; // KSAUDIO_SPEAKER_7POINT1_SURROUND
		RecordExtensibleSamplesPerSec(extensibleFormat);

		// 7.1 FRC_FLC_RR_RL_FC_LFE_FR_FL
		extensibleFormat->dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER;
		RecordExtensibleSamplesPerSec(extensibleFormat);

		break;
	}
}

void TestManager::RecordExtensibleSamplesPerSec(WAVEFORMATEXTENSIBLE* extensibleFormat)
{
	extensibleFormat->Format.nSamplesPerSec = 44100;
	RecordExtensibleBitsPerSample(extensibleFormat);

	extensibleFormat->Format.nSamplesPerSec = 48000;
	RecordExtensibleBitsPerSample(extensibleFormat);

	extensibleFormat->Format.nSamplesPerSec = 96000;
	RecordExtensibleBitsPerSample(extensibleFormat);

	extensibleFormat->Format.nSamplesPerSec = 192000;
	RecordExtensibleBitsPerSample(extensibleFormat);
}

void TestManager::RecordExtensibleBitsPerSample(WAVEFORMATEXTENSIBLE* extensibleFormat)
{
	SetBitsPerSample(&extensibleFormat->Format, 16);
	extensibleFormat->Samples.wValidBitsPerSample = extensibleFormat->Format.wBitsPerSample;
	RecordExtensibleFormat(extensibleFormat);

	SetBitsPerSample(&extensibleFormat->Format, 24);
	extensibleFormat->Samples.wValidBitsPerSample = extensibleFormat->Format.wBitsPerSample;
	RecordExtensibleFormat(extensibleFormat);

	SetBitsPerSample(&extensibleFormat->Format, 32);
	extensibleFormat->Samples.wValidBitsPerSample = extensibleFormat->Format.wBitsPerSample;
	RecordExtensibleFormat(extensibleFormat);
}

void TestManager::RecordExtensibleFormat(WAVEFORMATEXTENSIBLE* extensibleFormat)
{
	WAVEFORMATEXTENSIBLE* pWfx = new WAVEFORMATEXTENSIBLE();
	*pWfx = *extensibleFormat;
	AllHDMIFormats.push_back((WAVEFORMATEX*)pWfx);
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
