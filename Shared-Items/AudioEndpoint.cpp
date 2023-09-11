#include "AudioEndpoint.h"
#include <Audioclient.h>
#include "WindowsWaveFormats.h"
#include <algorithm>
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

AudioEndpoint::AudioEndpoint(IMMDevice * device, std::string name, std::string id) : Device(device), Name(name), ID(id)
{
    device->AddRef();
}

AudioEndpoint::~AudioEndpoint()
{
	SAFE_RELEASE(Device);
	ClearFormats();
}

void AudioEndpoint::ClearFormats()
{
	if (SupportedFormats.size() > 0)
	{
		for (SupportedAudioFormat* format : SupportedFormats)
		{
			delete format;
		}
		SupportedFormats.clear();
	}

	if (DuplicateSupportedFormats.size() > 0)
	{
		for (SupportedAudioFormat* format : DuplicateSupportedFormats)
		{
			delete format;
		}
		DuplicateSupportedFormats.clear();
	}
}

void AudioEndpoint::PopulateSupportedFormats(bool includeDuplicateFormats, bool includeSurroundAsDefault, bool ensureOneFormat, bool selectDefaults, bool (*formatFilter)(AudioFormat*))
{
	ClearFormats();

	const IID IID_IAudioClient = __uuidof(IAudioClient);
	IAudioClient* pAudioClient = NULL;
	HRESULT hr = Device->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&pAudioClient);

	if (!FAILED(hr))
	{
		// Favour formats that have channel masks
		for (AudioFormat* format : WindowsWaveFormats::Formats.AllExtensibleAudioFormats)
		{
			if (formatFilter(format))
			{
				WAVEFORMATEXTENSIBLE* waveFormat = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(format->GetWaveFormat());
				if (waveFormat->dwChannelMask != 0)
				{
					hr = pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, (WAVEFORMATEX*)waveFormat, NULL);
					if (hr == S_OK)
					{
						// this format is supported!
						SupportedFormats.push_back(new SupportedAudioFormat(format));
					}
				}
			}
		}
		// Next, look at formats that don't have channel masks
		for (AudioFormat* format : WindowsWaveFormats::Formats.AllExtensibleAudioFormats)
		{
			if (formatFilter(format))
			{
				WAVEFORMATEXTENSIBLE* waveFormat = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(format->GetWaveFormat());
				if (waveFormat->dwChannelMask == 0)
				{
					hr = pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, (WAVEFORMATEX*)waveFormat, NULL);
					if (hr == S_OK)
					{
						// this format is supported!

						// Only add formats with no channel masks if there are no supported formats with channel masks
						if (!AlreadyInSupportedFormats(AudioFormat::GetFormatID((WAVEFORMATEX*)waveFormat), waveFormat->Format.nChannels, waveFormat->Format.nSamplesPerSec, waveFormat->Format.wBitsPerSample))
						{
							SupportedFormats.push_back(new SupportedAudioFormat(format));
						}
						else if (includeDuplicateFormats)
						{
							DuplicateSupportedFormats.push_back(new SupportedAudioFormat(format));
						}
					}
				}
			}
		}
		for (AudioFormat* format : WindowsWaveFormats::Formats.AllExAudioFormats)
		{
			if (formatFilter(format))
			{
				hr = pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, format->GetWaveFormat(), NULL);
				if (hr == S_OK)
				{
					// this format is supported!
					WAVEFORMATEX* waveFormat = format->GetWaveFormat();

					// WAVEFORMATEX are legacy formats that are typically duplicates of the WAVEFORMATEXTENSIBLE ones
					// But they are still important because NVIDIA (and possibly other) drivers only support 24 bit HDMI
					// audio output through these legacy formats.
					if (!AlreadyInSupportedFormats(AudioFormat::GetFormatID(waveFormat), waveFormat->nChannels, waveFormat->nSamplesPerSec, waveFormat->wBitsPerSample))
					{
						SupportedFormats.push_back(new SupportedAudioFormat(format));
					}
					else if (includeDuplicateFormats)
					{
						DuplicateSupportedFormats.push_back(new SupportedAudioFormat(format));
					}
				}
			}
		}

		// Next, add on File audio formats:
		for (AudioFormat* format : FileAudioFormats::Formats.AllFileFormats)
		{
			if (formatFilter(format))
			{
				SupportedFormats.push_back(new SupportedAudioFormat(format));
			}
		}

		// Sort supported formats
		auto sortFunc = [](const SupportedAudioFormat* a, const SupportedAudioFormat* b) {
			if (a->Format->type == AudioFormat::FormatType::File && b->Format->type == AudioFormat::FormatType::File)
			{
				return false;
			}
			else if (a->Format->type == AudioFormat::FormatType::File && b->Format->type == AudioFormat::FormatType::WaveFormatEx)
			{
				return true;
			}
			else if (a->Format->type == AudioFormat::FormatType::WaveFormatEx && b->Format->type == AudioFormat::FormatType::File)
			{
				return false;
			}
			else
			{
				int aFormatOrder = GetFormatIdOrder(a->Format->GetWaveFormat());
				int bFormatOrder = GetFormatIdOrder(b->Format->GetWaveFormat());
				if (aFormatOrder != bFormatOrder)
				{
					return aFormatOrder < bFormatOrder;
				}
				else if (a->Format->GetWaveFormat()->nChannels != b->Format->GetWaveFormat()->nChannels)
				{
					return a->Format->GetWaveFormat()->nChannels < b->Format->GetWaveFormat()->nChannels;
				}
				else if (a->Format->GetWaveFormat()->nSamplesPerSec != b->Format->GetWaveFormat()->nSamplesPerSec)
				{
					return a->Format->GetWaveFormat()->nSamplesPerSec < b->Format->GetWaveFormat()->nSamplesPerSec;
				}
				else if (a->Format->GetWaveFormat()->wBitsPerSample != b->Format->GetWaveFormat()->wBitsPerSample)
				{
					return a->Format->GetWaveFormat()->wBitsPerSample < b->Format->GetWaveFormat()->wBitsPerSample;
				}
				else
				{
					return a->Format->FormatString < b->Format->FormatString;
				}
			}
		};

		std::sort(SupportedFormats.begin(), SupportedFormats.end(), sortFunc);
		std::sort(DuplicateSupportedFormats.begin(), DuplicateSupportedFormats.end(), sortFunc);

		SetDefaultFormats(includeSurroundAsDefault, ensureOneFormat, selectDefaults);
	}

	SAFE_RELEASE(pAudioClient);
}

int AudioEndpoint::GetFormatIdOrder(WAVEFORMATEX* waveFormat)
{
	WAVEFORMATEXTENSIBLE* waveFormatExtensible = nullptr;
	GUID subFormat;
	if (waveFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
	{
		waveFormatExtensible = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(waveFormat);
		subFormat = waveFormatExtensible->SubFormat;
	}
	WORD formatId = AudioFormat::GetFormatID(waveFormat);

	int order = 0;

	if (formatId == WAVE_FORMAT_PCM) { return order; }
	order++;
	if (formatId == WAVE_FORMAT_ADPCM) { return order; }
	order++;
	if (waveFormatExtensible != nullptr && subFormat == KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_DIGITAL_PLUS_ATMOS) { return order; }
	order++;
	if (waveFormatExtensible != nullptr && subFormat == KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_DIGITAL_PLUS) { return order; }
	order++;
	if (waveFormatExtensible != nullptr && subFormat == KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_DIGITAL) { return order; }
	order++;
	if (waveFormatExtensible != nullptr && subFormat == KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_MAT21) { return order; }
	order++;
	if (waveFormatExtensible != nullptr && subFormat == KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_MAT20) { return order; }
	order++;
	if (waveFormatExtensible != nullptr && subFormat == KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_MLP) { return order; }
	order++;
	if (formatId == WAVE_FORMAT_DOLBY_AC4) { return order; }
	order++;
	if (formatId == WAVE_FORMAT_DOLBY_AC3_SPDIF) { return order; }
	order++;
	if (formatId == WAVE_FORMAT_DOLBY_AC2) { return order; }
	order++;
	if (waveFormatExtensible != nullptr && subFormat == KSDATAFORMAT_SUBTYPE_IEC61937_DTSX_E2) { return order; }
	order++;
	if (waveFormatExtensible != nullptr && subFormat == KSDATAFORMAT_SUBTYPE_IEC61937_DTSX_E1) { return order; }
	order++;
	if (waveFormatExtensible != nullptr && subFormat == KSDATAFORMAT_SUBTYPE_IEC61937_DTS_HD) { return order; }
	order++;
	if (waveFormatExtensible != nullptr && subFormat == KSDATAFORMAT_SUBTYPE_IEC61937_DTS) { return order; }
	order++;
	if (waveFormatExtensible != nullptr && subFormat == KSDATAFORMAT_SUBTYPE_DTS_AUDIO) { return order; }
	order++;
	if (formatId == WAVE_FORMAT_DTS2) { return order; }
	order++;
	if (formatId == WAVE_FORMAT_DTS_DS) { return order; }
	order++;
	if (formatId == WAVE_FORMAT_DTS2) { return order; }
	order++;
	if (formatId == WAVE_FORMAT_IEEE_FLOAT) { return order; }
	order++;
	if (formatId == WAVE_FORMAT_ALAW) { return order; }
	order++;
	if (formatId == WAVE_FORMAT_MULAW) { return order; }
	order++;

	return order;
}

void AudioEndpoint::SetDefaultFormats(bool includeSurroundAsDefault, bool ensureOneFormat, bool selectDefaults)
{
	bool foundFormat = true;

	std::vector<SupportedAudioFormat*> stereoFormats = GetWaveFormatExFormats(2, 48000, 16);
	if (stereoFormats.size() > 1)
	{
		// This means there are multiple formats that all have non-zero channel masks, so pick the channel mask we care about
		for (SupportedAudioFormat* format : stereoFormats)
		{
			if (format->Format->GetWaveFormat()->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
			{
				if (reinterpret_cast<WAVEFORMATEXTENSIBLE*>(format->Format->GetWaveFormat())->dwChannelMask == (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT))
				{
					format->DefaultSelection = true;
					if (selectDefaults)
					{
						format->UserSelected = true;
					}
				}
			}
		}
	}
	else if (stereoFormats.size() > 0)
	{
		stereoFormats[0]->DefaultSelection = true;
		if (selectDefaults)
		{
			stereoFormats[0]->UserSelected = true;
		}
	}
	else if (ensureOneFormat)
	{
		// We didn't find any, but maybe there are some at a different sample rate or bit rate:
		std::vector<SupportedAudioFormat*> stereoFormats = GetWaveFormatExFormats(2);
		if (stereoFormats.size() > 0)
		{
			// We didn't get our preference as described above, so just go with whatever one
			// is first on the list. This is scenario happens when using the Analog audio latency type.
			stereoFormats[0]->DefaultSelection = true;
			if (selectDefaults)
			{
				stereoFormats[0]->UserSelected = true;
			}
		}
		else
		{
			foundFormat = false;
		}
	}

	if (includeSurroundAsDefault)
	{
		std::vector<SupportedAudioFormat*> fivePointOneFormats = GetWaveFormatExFormats(6, 48000, 16);
		if (fivePointOneFormats.size() > 1)
		{
			bool foundIt = false;
			// This means there are multiple formats that all have non-zero channel masks, so pick the channel mask we care about
			for (SupportedAudioFormat* format : fivePointOneFormats)
			{
				if (format->Format->GetWaveFormat()->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
				{
					// KSAUDIO_SPEAKER_5POINT1_SURROUND (using side/surround speakers)
					if (reinterpret_cast<WAVEFORMATEXTENSIBLE*>(format->Format->GetWaveFormat())->dwChannelMask == (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT))
					{
						foundFormat = true;
						format->DefaultSelection = true;
						if (selectDefaults)
						{
							format->UserSelected = true;
						}
						foundIt = true;
					}
				}
			}
			if (!foundIt)
			{
				for (SupportedAudioFormat* format : fivePointOneFormats)
				{
					if (format->Format->GetWaveFormat()->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
					{
						// KSAUDIO_SPEAKER_5POINT1 (using rear left/right of center speakers)
						// Note: This format is excluded for HDMI, so this will never happen for HDMI formats.
						if (reinterpret_cast<WAVEFORMATEXTENSIBLE*>(format->Format->GetWaveFormat())->dwChannelMask == (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT))
						{
							foundFormat = true;
							format->DefaultSelection = true;
							if (selectDefaults)
							{
								format->UserSelected = true;
							}
						}
					}
				}
			}
		}
		else if (fivePointOneFormats.size() > 0)
		{
			foundFormat = true;
			fivePointOneFormats[0]->DefaultSelection = true;
			if (selectDefaults)
			{
				fivePointOneFormats[0]->UserSelected = true;
			}
		}

		std::vector<SupportedAudioFormat*> sevenPointOneFormats = GetWaveFormatExFormats(8, 48000, 16);
		if (sevenPointOneFormats.size() > 1)
		{
			// This means there are multiple formats that all have non-zero channel masks, so pick the channel mask we care about
			for (SupportedAudioFormat* format : sevenPointOneFormats)
			{
				if (format->Format->GetWaveFormat()->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
				{
					// KSAUDIO_SPEAKER_7POINT1_SURROUND
					if (reinterpret_cast<WAVEFORMATEXTENSIBLE*>(format->Format->GetWaveFormat())->dwChannelMask == (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT))
					{
						foundFormat = true;
						format->DefaultSelection = true;
						if (selectDefaults)
						{
							format->UserSelected = true;
						}
					}
				}
			}
		}
		else if (sevenPointOneFormats.size() > 0)
		{
			foundFormat = true;
			sevenPointOneFormats[0]->DefaultSelection = true;
			if (selectDefaults)
			{
				sevenPointOneFormats[0]->UserSelected = true;
			}
		}
	}

	if (ensureOneFormat && !foundFormat)
	{
		if (SupportedFormats.size() > 0)
		{
			SupportedFormats[0]->DefaultSelection = true;
			if (selectDefaults)
			{
				SupportedFormats[0]->UserSelected = true;
			}
		}
		else
		{
			printf("Error: could not find audio format.");
		}
	}
}

bool AudioEndpoint::AlreadyInSupportedFormats(WORD formatId, int numChannels, int samplesPerSec, int bitsPerSample)
{
	if (formatId != WAVE_FORMAT_PCM
		&& formatId != WAVE_FORMAT_IEEE_FLOAT)
	{
		// This means that it is likely an encoded format.
		// Encoded formats do not have duplicates because they are manually configured to match
		// what the audio encoder is able to produce. This means we can simply assume that this
		// format is not already in the supported formats list.
		return false;
	}
	for (SupportedAudioFormat* audioFormat : SupportedFormats)
	{
		if (audioFormat->Format->type == AudioFormat::FormatType::WaveFormatEx
			&& audioFormat->Format->GetWaveFormat()->nChannels == numChannels
			&& audioFormat->Format->GetWaveFormat()->nSamplesPerSec == samplesPerSec
			&& audioFormat->Format->GetWaveFormat()->wBitsPerSample == bitsPerSample
			&& AudioFormat::GetFormatID(audioFormat->Format->GetWaveFormat()) == formatId)
		{
			return true;
		}
	}
	return false;
}

std::vector<SupportedAudioFormat*> AudioEndpoint::GetWaveFormatExFormats(int numChannels, int samplesPerSec, int bitsPerSample)
{
	std::vector<SupportedAudioFormat*> result;
	for (SupportedAudioFormat* audioFormat : SupportedFormats)
	{
		if (audioFormat->Format->type == AudioFormat::FormatType::WaveFormatEx
			&& audioFormat->Format->GetWaveFormat()->nChannels == numChannels
			&& audioFormat->Format->GetWaveFormat()->nSamplesPerSec == samplesPerSec
			&& audioFormat->Format->GetWaveFormat()->wBitsPerSample == bitsPerSample)
		{
			result.push_back(audioFormat);
		}
	}
	return result;
}

std::vector<SupportedAudioFormat*> AudioEndpoint::GetWaveFormatExFormats(int numChannels, int samplesPerSec)
{
	std::vector<SupportedAudioFormat*> result;
	for (SupportedAudioFormat* audioFormat : SupportedFormats)
	{
		if (audioFormat->Format->type == AudioFormat::FormatType::WaveFormatEx
			&& audioFormat->Format->GetWaveFormat()->nChannels == numChannels
			&& audioFormat->Format->GetWaveFormat()->nSamplesPerSec == samplesPerSec)
		{
			result.push_back(audioFormat);
		}
	}
	return result;
}

std::vector<SupportedAudioFormat*> AudioEndpoint::GetWaveFormatExFormats(int numChannels)
{
	std::vector<SupportedAudioFormat*> result;
	for (SupportedAudioFormat* audioFormat : SupportedFormats)
	{
		if (audioFormat->Format->type == AudioFormat::FormatType::WaveFormatEx
			&& audioFormat->Format->GetWaveFormat()->nChannels == numChannels)
		{
			result.push_back(audioFormat);
		}
	}
	return result;
}
