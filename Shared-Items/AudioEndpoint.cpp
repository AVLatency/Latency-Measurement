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
				if (!SupportsExtensibleFormat(waveFormat->Format.nChannels, waveFormat->Format.nSamplesPerSec, waveFormat->Format.wBitsPerSample))
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
							// Only add formats with no channel masks if there are no supported formats with channel masks
			if (!SupportsExtensibleFormat(waveFormat->nChannels, waveFormat->nSamplesPerSec))
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
	}

	SAFE_RELEASE(pAudioClient);
}

bool AudioEndpoint::SupportsExtensibleFormat(int numChannels, int samplesPerSec, int bitsPerSample)
{
	for (AudioFormat audioFormat : SupportedFormats)
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

bool AudioEndpoint::SupportsExtensibleFormat(int numChannels, int samplesPerSec)
{
	for (AudioFormat audioFormat : SupportedFormats)
	{
		if (audioFormat.WaveFormat->nChannels == numChannels
			&& audioFormat.WaveFormat->nSamplesPerSec == samplesPerSec)
		{
			return true;
		}
	}
	return false;
}
