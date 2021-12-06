#include "AudioEndpoint.h"
#include <Audioclient.h>
#include "HdmiWaveFormats.h"
#include <algorithm>
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

AudioEndpoint::AudioEndpoint(IMMDevice * device, std::string name, std::string id) : Device(device), Name(name), ID(id)
{
    device->AddRef();
}

AudioEndpoint::AudioEndpoint(const AudioEndpoint & other)
{
    Device = other.Device;
    Device->AddRef();
    Name = other.Name;
    ID = other.ID;
}

AudioEndpoint::AudioEndpoint(const AudioEndpoint&& other)
{
    Device = other.Device;
    Device->AddRef();
    Name = other.Name;
    ID = other.ID;
}

AudioEndpoint::~AudioEndpoint()
{
	SAFE_RELEASE(Device);
}

void AudioEndpoint::PopulateSupportedFormats()
{
	if (SupportedFormats.size() > 0)
	{
		return;
	}

	const IID IID_IAudioClient = __uuidof(IAudioClient);
	IAudioClient* pAudioClient = NULL;
	HRESULT hr = Device->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&pAudioClient);

	if (!FAILED(hr))
	{
		// Favour formats that have channel masks
		for (WAVEFORMATEXTENSIBLE* waveFormat : HdmiWaveFormats::Formats.AllHDMIExtensibleFormats)
		{
			if (waveFormat->dwChannelMask != 0)
			{
				hr = pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, (WAVEFORMATEX*)waveFormat, NULL);
				if (hr == S_OK)
				{
					// this format is supported!
					SupportedFormats.push_back((WAVEFORMATEX*)waveFormat);
				}
			}
		}
		// Next, look at formats that don't have channel masks
		for (WAVEFORMATEXTENSIBLE* waveFormat : HdmiWaveFormats::Formats.AllHDMIExtensibleFormats)
		{
			if (waveFormat->dwChannelMask == 0)
			{
				// Only add formats with no channel masks if there are no supported formats with channel masks
				if (!SupportsFormat(waveFormat->Format.nChannels, waveFormat->Format.nSamplesPerSec, waveFormat->Format.wBitsPerSample))
				{
					hr = pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, (WAVEFORMATEX*)waveFormat, NULL);
					if (hr == S_OK)
					{
						// this format is supported!
						SupportedFormats.push_back((WAVEFORMATEX*)waveFormat);
					}
				}
			}
		}
		for (WAVEFORMATEX* waveFormat : HdmiWaveFormats::Formats.AllHDMIExFormats)
		{
			// WAVEFORMATEX are legacy formats that are typically duplicates of the WAVEFORMATEXTENSIBLE ones.
			// Furthermore, WAVEFORMATEX formats' bit depth property are not respected on NVIDIA drivers
			// (24 bit WAVEFORMATEX actually produce a 16 bit HDMI signal)
			// For these reasons, ignore any WAVEFORMATEX formats that are duplicates of WAVEFORMATEXTENSIBLE ones.

			// TODO: Verify using an HDMI analyzer that this bit about bit depths is true for NVIDA, AMD, and Intel audio drivers.
			
			if (!SupportsFormat(waveFormat->nChannels, waveFormat->nSamplesPerSec))
			{
				hr = pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, waveFormat, NULL);
				if (hr == S_OK)
				{
					// this format is supported!
					SupportedFormats.push_back(waveFormat);
				}
			}
		}

		// Sort supported formats
		std::sort(SupportedFormats.begin(), SupportedFormats.end(), [](const AudioFormat& a, const AudioFormat& b) {
			if (a.WaveFormat->nChannels != b.WaveFormat->nChannels)
			{
				return a.WaveFormat->nChannels < b.WaveFormat->nChannels;
			}
			else if (a.WaveFormat->nSamplesPerSec != b.WaveFormat->nSamplesPerSec)
			{
				return a.WaveFormat->nSamplesPerSec < b.WaveFormat->nSamplesPerSec;
			}
			else if (a.WaveFormat->wBitsPerSample != b.WaveFormat->wBitsPerSample)
			{
				return a.WaveFormat->wBitsPerSample < b.WaveFormat->wBitsPerSample;
			}
			else
			{
				return a.FormatString < b.FormatString;
			}
		});

		SelectDefaultFormats();
	}

	SAFE_RELEASE(pAudioClient);
}

void AudioEndpoint::SelectDefaultFormats()
{
	std::vector<AudioFormat*> stereoFormats = GetFormats(2, 48000, 16);
	if (stereoFormats.size() > 1)
	{
		// This means there are multiple formats that all have non-zero channel masks, so pick the channel mask we care about
		for (AudioFormat* format : stereoFormats)
		{
			if (format->WaveFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
			{
				if (reinterpret_cast<WAVEFORMATEXTENSIBLE*>(format->WaveFormat)->dwChannelMask == (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT))
				{
					format->UserSelected = true;
				}
			}
		}
	}
	else if (stereoFormats.size() > 0)
	{
		stereoFormats[0]->UserSelected = true;
	}

	std::vector<AudioFormat*> fivePointOneFormats = GetFormats(6, 48000, 16);
	if (fivePointOneFormats.size() > 1)
	{
		bool foundIt = false;
		// This means there are multiple formats that all have non-zero channel masks, so pick the channel mask we care about
		for (AudioFormat* format : fivePointOneFormats)
		{
			if (format->WaveFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
			{
				// KSAUDIO_SPEAKER_5POINT1_SURROUND (using side/surround speakers)
				if (reinterpret_cast<WAVEFORMATEXTENSIBLE*>(format->WaveFormat)->dwChannelMask == (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT))
				{
					format->UserSelected = true;
					foundIt = true;
				}
			}
		}
		if (!foundIt)
		{
			for (AudioFormat* format : fivePointOneFormats)
			{
				if (format->WaveFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
				{
					// KSAUDIO_SPEAKER_5POINT1 (using rear left/right of center speakers)
					if (reinterpret_cast<WAVEFORMATEXTENSIBLE*>(format->WaveFormat)->dwChannelMask == (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT))
					{
						format->UserSelected = true;
					}
				}
			}
		}
	}
	else if (fivePointOneFormats.size() > 0)
	{
		fivePointOneFormats[0]->UserSelected = true;
	}

	std::vector<AudioFormat*> sevenPointOneFormats = GetFormats(8, 48000, 16);
	if (sevenPointOneFormats.size() > 1)
	{
		// This means there are multiple formats that all have non-zero channel masks, so pick the channel mask we care about
		for (AudioFormat* format : sevenPointOneFormats)
		{
			if (format->WaveFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
			{
				// KSAUDIO_SPEAKER_7POINT1_SURROUND
				if (reinterpret_cast<WAVEFORMATEXTENSIBLE*>(format->WaveFormat)->dwChannelMask == (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT))
				{
					format->UserSelected = true;
				}
			}
		}
	}
	else if (sevenPointOneFormats.size() > 0)
	{
		sevenPointOneFormats[0]->UserSelected = true;
	}
}

bool AudioEndpoint::SupportsFormat(int numChannels, int samplesPerSec, int bitsPerSample)
{
	for (AudioFormat& audioFormat : SupportedFormats)
	{
		if (audioFormat.WaveFormat->nChannels == numChannels
			&& audioFormat.WaveFormat->nSamplesPerSec == samplesPerSec
			&& audioFormat.WaveFormat->wBitsPerSample == bitsPerSample)
		{
			return true;
		}
	}
	return false;
}

bool AudioEndpoint::SupportsFormat(int numChannels, int samplesPerSec)
{
	for (AudioFormat& audioFormat : SupportedFormats)
	{
		if (audioFormat.WaveFormat->nChannels == numChannels
			&& audioFormat.WaveFormat->nSamplesPerSec == samplesPerSec)
		{
			return true;
		}
	}
	return false;
}

std::vector<AudioFormat*> AudioEndpoint::GetFormats(int numChannels, int samplesPerSec, int bitsPerSample)
{
	std::vector<AudioFormat*> result;
	for (AudioFormat& audioFormat : SupportedFormats)
	{
		if (audioFormat.WaveFormat->nChannels == numChannels
			&& audioFormat.WaveFormat->nSamplesPerSec == samplesPerSec
			&& audioFormat.WaveFormat->wBitsPerSample == bitsPerSample)
		{
			result.push_back(&audioFormat);
		}
	}
	return result;
}

std::vector<AudioFormat*> AudioEndpoint::GetFormats(int numChannels, int samplesPerSec)
{
	std::vector<AudioFormat*> result;
	for (AudioFormat& audioFormat : SupportedFormats)
	{
		if (audioFormat.WaveFormat->nChannels == numChannels
			&& audioFormat.WaveFormat->nSamplesPerSec == samplesPerSec)
		{
			result.push_back(&audioFormat);
		}
	}
	return result;
}

std::vector<AudioFormat*> AudioEndpoint::GetFormats(int numChannels)
{
	std::vector<AudioFormat*> result;
	for (AudioFormat& audioFormat : SupportedFormats)
	{
		if (audioFormat.WaveFormat->nChannels == numChannels)
		{
			result.push_back(&audioFormat);
		}
	}
	return result;
}
