#include "OutputOffsetProfiles.h"
#include "WindowsWaveFormats.h"
#include <sstream>
#include <format>

OutputOffsetProfile* OutputOffsetProfiles::Hdmi_HDV_MB01 = NULL;
OutputOffsetProfile* OutputOffsetProfiles::Hdmi_None = NULL;
OutputOffsetProfile* OutputOffsetProfiles::Hdmi_None_CurrentWinAudioFormat = NULL;

OutputOffsetProfile* OutputOffsetProfiles::Spdif_HDV_MB01 = NULL;
OutputOffsetProfile* OutputOffsetProfiles::Spdif_AYSA11 = NULL;
OutputOffsetProfile* OutputOffsetProfiles::Spdif_LiNKFOR_USB_DAC = NULL;
OutputOffsetProfile* OutputOffsetProfiles::Spdif_None = NULL;
OutputOffsetProfile* OutputOffsetProfiles::Spdif_None_CurrentWinAudioFormat = NULL;

OutputOffsetProfile* OutputOffsetProfiles::Arc_ExampleReferenceSetup = NULL;
OutputOffsetProfile* OutputOffsetProfiles::Arc_None = NULL;
OutputOffsetProfile* OutputOffsetProfiles::Arc_None_CurrentWinAudioFormat = NULL;

OutputOffsetProfile* OutputOffsetProfiles::EArc_ExampleReferenceSetup = NULL;
OutputOffsetProfile* OutputOffsetProfiles::EArc_None = NULL;
OutputOffsetProfile* OutputOffsetProfiles::EArc_None_CurrentWinAudioFormat = NULL;

OutputOffsetProfile* OutputOffsetProfiles::Analog_BasicYSplitter = NULL;
OutputOffsetProfile* OutputOffsetProfiles::Analog_None = NULL;

OutputOffsetProfile* OutputOffsetProfiles::HdmiAudioPassthrough_HDV_MB01 = NULL;
OutputOffsetProfile* OutputOffsetProfiles::HdmiAudioPassthrough_None = NULL;
OutputOffsetProfile* OutputOffsetProfiles::HdmiAudioPassthrough_None_CurrentWinAudioFormat = NULL;

OutputOffsetProfile* OutputOffsetProfiles::RelativeWinAudio_Wasapi = NULL;

std::vector<OutputOffsetProfile*> OutputOffsetProfiles::Profiles;
int OutputOffsetProfiles::SelectedProfileIndex = 0;

std::map<OutputOffsetProfile::OutputType, OutputOffsetProfiles::ProfilesSubset*> OutputOffsetProfiles::Subsets;

void OutputOffsetProfiles::InitializeProfiles(ProfileResources& resources)
{
	// *****************************************************
	// HDMI AUDIO LATENCY PROFILES
	// *****************************************************

	Hdmi_HDV_MB01 = new OutputOffsetProfile(OutputOffsetProfile::OutputType::Hdmi, "HDV-MB01", Hdmi_HDV_MB01_GetOffset, None_GetOffset, HdmiLpcmFormatsFilter);
	Hdmi_HDV_MB01->Image = resources.Hdmi_HDV_MB01_Texture;
	Hdmi_HDV_MB01->Description =
		"The HDV-MB01 is sold under these names:\n\n"
		
		"- J-Tech Digital JTD18G - H5CH\n"
		"- Monoprice Blackbird 24278\n"
		"- OREI HDA - 912";
	Profiles.push_back(Hdmi_HDV_MB01);

	Hdmi_None = new OutputOffsetProfile(OutputOffsetProfile::OutputType::Hdmi, "Other", None_GetOffset, None_GetOffset, HdmiAllFormatsFilter);
	Hdmi_None->isNoOffset = true;
	Profiles.push_back(Hdmi_None);

	Hdmi_None_CurrentWinAudioFormat = new OutputOffsetProfile(OutputOffsetProfile::OutputType::Hdmi, "Other (Current Windows audio format)", None_GetOffset, None_GetOffset, AllFormatsFilter);
	Hdmi_None_CurrentWinAudioFormat->isNoOffset = true;
	Hdmi_None_CurrentWinAudioFormat->isCurrentWindowsAudioFormat = true;
	Profiles.push_back(Hdmi_None_CurrentWinAudioFormat);

	// *****************************************************
	// S/PDIF AUDIO LATENCY PROFILES
	// *****************************************************

	Spdif_HDV_MB01 = new OutputOffsetProfile(OutputOffsetProfile::OutputType::Spdif, "HDV-MB01", Spdif_HDV_MB01_GetOffset, None_GetOffset, SpdifLpcmFormatFilter);
	Spdif_HDV_MB01->Image = resources.Spdif_HDV_MB01_Texture;
	Spdif_HDV_MB01->Description =
		"The HDV-MB01 is sold under these names:\n\n"
		
		"- J-Tech Digital JTD18G - H5CH\n"
		"- Monoprice Blackbird 24278\n"
		"- OREI HDA - 912\n\n"
		
		"Supported S/PDIF Formats:\n\n"

		"2ch-44.1kHz-16bit\n"
		"2ch-44.1kHz-24bit\n"
		"2ch-48kHz-16bit\n"
		"2ch-48kHz-24bit\n"
		"2ch-96kHz-16bit\n"
		"2ch-96kHz-24bit\n"
		"2ch-192kHz-16bit\n"
		"2ch-192kHz-24bit\n";
	Profiles.push_back(Spdif_HDV_MB01);

	Spdif_AYSA11 = new OutputOffsetProfile(OutputOffsetProfile::OutputType::Spdif, "AYSA11", Spdif_AYSA11_GetOffset, None_GetOffset, SpdifLpcmFormatFilter);
	Spdif_AYSA11->Description =
		"Supported S/PDIF Formats:\n\n"

		"2ch-44.1kHz-16bit\n"
		"2ch-44.1kHz-24bit\n"
		"2ch-48kHz-16bit\n"
		"2ch-48kHz-24bit\n"
		"2ch-96kHz-16bit\n"
		"2ch-96kHz-24bit\n"
		"2ch-192kHz-16bit\n"
		"2ch-192kHz-24bit\n";
	Profiles.push_back(Spdif_AYSA11);

	Spdif_LiNKFOR_USB_DAC = new OutputOffsetProfile(OutputOffsetProfile::OutputType::Spdif, "LiNKFOR USB DAC", Spdif_LiNKFOR_USB_DAC_GetOffset, None_GetOffset, SpdifLpcmFormatFilter);
	Spdif_LiNKFOR_USB_DAC->Description =
		"Supported S/PDIF Formats:\n\n"

		"2ch-44.1kHz-16bit\n"
		"2ch-48kHz-16bit\n";
	Profiles.push_back(Spdif_LiNKFOR_USB_DAC);

	Spdif_None = new OutputOffsetProfile(OutputOffsetProfile::OutputType::Spdif, "Other", None_GetOffset, None_GetOffset, SpdifAllFormatFilter);
	Spdif_None->isNoOffset = true;
	Profiles.push_back(Spdif_None);

	Spdif_None_CurrentWinAudioFormat = new OutputOffsetProfile(OutputOffsetProfile::OutputType::Spdif, "Other (Current Windows audio format)", None_GetOffset, None_GetOffset, AllFormatsFilter);
	Spdif_None_CurrentWinAudioFormat->isNoOffset = true;
	Spdif_None_CurrentWinAudioFormat->isCurrentWindowsAudioFormat = true;
	Profiles.push_back(Spdif_None_CurrentWinAudioFormat);

	// *****************************************************
	// ARC AUDIO LATENCY PROFILES
	// *****************************************************

#ifdef _DEBUG
	Arc_ExampleReferenceSetup = new OutputOffsetProfile(OutputOffsetProfile::OutputType::ARC, "Example ARC Reference Setup", Arc_ExampleReferenceSetup_GetOffset, None_GetOffset, AllFormatsFilter);
	Arc_ExampleReferenceSetup->Image = resources.Arc_ExampleDevice_Texture;
	Arc_ExampleReferenceSetup->Description =
		"This is a DEBUG-only example ARC Receiver Latency reference device/setup that can be used as a starting point for creating new Output Offset Profiles.";
	Profiles.push_back(Arc_ExampleReferenceSetup);
#endif //_DEBUG

	Arc_None = new OutputOffsetProfile(OutputOffsetProfile::OutputType::ARC, "Other", None_GetOffset, None_GetOffset, AllFormatsFilter);
	Arc_None->isNoOffset = true;
	Profiles.push_back(Arc_None);

	Arc_None_CurrentWinAudioFormat = new OutputOffsetProfile(OutputOffsetProfile::OutputType::ARC, "Other (Current Windows audio format)", None_GetOffset, None_GetOffset, AllFormatsFilter);
	Arc_None_CurrentWinAudioFormat->isNoOffset = true;
	Arc_None_CurrentWinAudioFormat->isCurrentWindowsAudioFormat = true;
	Profiles.push_back(Arc_None_CurrentWinAudioFormat);

	// *****************************************************
	// EARC AUDIO LATENCY PROFILES
	// *****************************************************

#ifdef _DEBUG
	EArc_ExampleReferenceSetup = new OutputOffsetProfile(OutputOffsetProfile::OutputType::eARC, "Example eARC Reference Setup", EArc_ExampleReferenceSetup_GetOffset, None_GetOffset, AllFormatsFilter);
	EArc_ExampleReferenceSetup->Image = resources.EArc_ExampleDevice_Texture;
	EArc_ExampleReferenceSetup->Description =
		"This is a DEBUG-only example eARC Receiver Latency reference device/setup that can be used as a starting point for creating new Output Offset Profiles.";
	Profiles.push_back(EArc_ExampleReferenceSetup);
#endif //_DEBUG

	EArc_None = new OutputOffsetProfile(OutputOffsetProfile::OutputType::eARC, "Other", None_GetOffset, None_GetOffset, AllFormatsFilter);
	EArc_None->isNoOffset = true;
	Profiles.push_back(EArc_None);

	EArc_None_CurrentWinAudioFormat = new OutputOffsetProfile(OutputOffsetProfile::OutputType::eARC, "Other (Current Windows audio format)", None_GetOffset, None_GetOffset, AllFormatsFilter);
	EArc_None_CurrentWinAudioFormat->isNoOffset = true;
	EArc_None_CurrentWinAudioFormat->isCurrentWindowsAudioFormat = true;
	Profiles.push_back(EArc_None_CurrentWinAudioFormat);

	// *****************************************************
	// ANALOG AUDIO LATENCY PROFILES
	// *****************************************************

	Analog_BasicYSplitter = new OutputOffsetProfile(OutputOffsetProfile::OutputType::Analog, "Basic Analog Y Splitter", Analog_BasicYSplitter_GetOffset, None_GetOffset, AllFormatsFilter);
	Analog_BasicYSplitter->Image = resources.Analog_YSplitter_Texture;
	Analog_BasicYSplitter->Description =
		"For measuring analog audio latency, a basic analog Y splitter can be used to produce "
		"two identical outputs that can act as a \"Dual-Out Reference\".";
	Profiles.push_back(Analog_BasicYSplitter);

	Analog_None = new OutputOffsetProfile(OutputOffsetProfile::OutputType::Analog, "Other", None_GetOffset, None_GetOffset, AllFormatsFilter);
	Analog_None->isNoOffset = true;
	Profiles.push_back(Analog_None);

	// *****************************************************
	// HDMI AUDIO PASSTHROUGH LATENCY PROFILES
	// *****************************************************

	// A copy of the Hdmi_HDV_MB01 profile, but with OutputType::HdmiAudioPassthrough
	HdmiAudioPassthrough_HDV_MB01 = new OutputOffsetProfile(OutputOffsetProfile::OutputType::HdmiAudioPassthrough, "HDV-MB01", Hdmi_HDV_MB01_GetOffset, None_GetOffset, HdmiLpcmFormatsFilter);
	HdmiAudioPassthrough_HDV_MB01->Image = Hdmi_HDV_MB01->Image;
	HdmiAudioPassthrough_HDV_MB01->Description = Hdmi_HDV_MB01->Description;
	Profiles.push_back(HdmiAudioPassthrough_HDV_MB01);

	HdmiAudioPassthrough_None = new OutputOffsetProfile(OutputOffsetProfile::OutputType::HdmiAudioPassthrough, "Other", None_GetOffset, None_GetOffset, HdmiAllFormatsFilter);
	HdmiAudioPassthrough_None->isNoOffset = true;
	Profiles.push_back(HdmiAudioPassthrough_None);

	HdmiAudioPassthrough_None_CurrentWinAudioFormat = new OutputOffsetProfile(OutputOffsetProfile::OutputType::HdmiAudioPassthrough, "Other (Current Windows audio format)", None_GetOffset, None_GetOffset, AllFormatsFilter);
	HdmiAudioPassthrough_None_CurrentWinAudioFormat->isNoOffset = true;
	HdmiAudioPassthrough_None_CurrentWinAudioFormat->isCurrentWindowsAudioFormat = true;
	Profiles.push_back(HdmiAudioPassthrough_None_CurrentWinAudioFormat);

	RelativeWinAudio_Wasapi = new OutputOffsetProfile(OutputOffsetProfile::OutputType::RelativeWinAudio, "WASAPI Exclusive", None_GetOffset, None_GetOffset, AllFormatsFilter);
	Profiles.push_back(RelativeWinAudio_Wasapi);

	PrepareSubsetListsForGui();
	PrepareOffsetStringsForGui();
}

OutputOffsetProfile* OutputOffsetProfiles::CurrentProfile()
{
	return Profiles[SelectedProfileIndex];
}

void OutputOffsetProfiles::PrepareSubsetListsForGui()
{
	for (int i = 0; i < Profiles.size(); i++)
	{
		if (!Subsets.contains(Profiles[i]->OutType))
		{
			Subsets[Profiles[i]->OutType] = new ProfilesSubset();
		}
		Subsets[Profiles[i]->OutType]->ProfileIndeces.push_back(i);
	}
}

void OutputOffsetProfiles::PrepareOffsetStringsForGui()
{
	for (int i = 0; i < Profiles.size(); i++)
	{
		OutputOffsetProfile* profile = Profiles[i];
		if (profile->isCurrentWindowsAudioFormat)
		{
			OutputOffsetProfile::OutputOffset offset = profile->GetOffsetForCurrentWinFormat();
			if (offset.verified)
			{
				std::string highlightedOffsetValue = std::format("{}: {} ms", AudioFormat::GetCurrentWinAudioFormatString(), offset.value);
				profile->HighlightedVerifiedOffsetsForDisplay.push_back(highlightedOffsetValue);
			}

			AddOffsetStrToProfileForFormat(profile, nullptr);
		}
		else
		{
			OutputOffsetProfile::OutputOffset offset = profile->GetLpcmOffset(2, 48000, 16);
			if (offset.verified)
			{
				std::string highlightedOffsetValue = std::format("LPCM 2ch-48kHz-16bit: {} ms", offset.value);
				profile->HighlightedVerifiedOffsetsForDisplay.push_back(highlightedOffsetValue);
			}
			offset = profile->GetLpcmOffset(2, 192000, 16);
			if (offset.verified)
			{
				std::string highlightedOffsetValue = std::format("LPCM 2ch-192kHz-16bit: {} ms", offset.value);
				profile->HighlightedVerifiedOffsetsForDisplay.push_back(highlightedOffsetValue);
			}

			offset = profile->GetLpcmOffset(6, 48000, 16);
			if (offset.verified)
			{
				std::string highlightedOffsetValue = std::format("LPCM 6ch-48kHz-16bit: {} ms", offset.value);
				profile->HighlightedVerifiedOffsetsForDisplay.push_back(highlightedOffsetValue);
			}
			offset = profile->GetLpcmOffset(6, 192000, 16);
			if (offset.verified)
			{
				std::string highlightedOffsetValue = std::format("LPCM 6ch-192kHz-16bit: {} ms", offset.value);
				profile->HighlightedVerifiedOffsetsForDisplay.push_back(highlightedOffsetValue);
			}

			offset = profile->GetLpcmOffset(6, 48000, 16);
			if (offset.verified)
			{
				std::string highlightedOffsetValue = std::format("LPCM 8ch-48kHz-16bit: {} ms", offset.value);
				profile->HighlightedVerifiedOffsetsForDisplay.push_back(highlightedOffsetValue);
			}
			offset = profile->GetLpcmOffset(6, 192000, 16);
			if (offset.verified)
			{
				std::string highlightedOffsetValue = std::format("LPCM 8ch-192kHz-16bit: {} ms", offset.value);
				profile->HighlightedVerifiedOffsetsForDisplay.push_back(highlightedOffsetValue);
			}

			for (AudioFormat* format : FileAudioFormats::Formats.AllFileFormats)
			{
				AddOffsetStrToProfileForFormat(profile, format);
			}

			for (AudioFormat* format : WindowsWaveFormats::Formats.AllExAudioFormats)
			{
				AddOffsetStrToProfileForFormat(profile, format);
			}
			for (AudioFormat* format : WindowsWaveFormats::Formats.AllExtensibleAudioFormats)
			{
				if (format->type == AudioFormat::FormatType::WaveFormatEx)
				{
					auto formatId = AudioFormat::GetFormatID(format->GetWaveFormat());
					if (formatId == WAVE_FORMAT_PCM || formatId == WAVE_FORMAT_IEEE_FLOAT)
					{
						// PCM and FLOAT are handled by the previous for loop
						continue;
					}
				}

				// Since it's not PCM or FLOAT, that means its probably an encoded audio format, so we should add that to the list:
				AddOffsetStrToProfileForFormat(profile, format);
			}
		}
	}
}

void OutputOffsetProfiles::AddOffsetStrToProfileForFormat(OutputOffsetProfile* profile, AudioFormat* format)
{
	std::string audioFormatStr;
	if (profile->isCurrentWindowsAudioFormat)
	{
		audioFormatStr = AudioFormat::GetCurrentWinAudioFormatString();
	}
	else
	{
		audioFormatStr = format->FormatString;
	}

	if (profile->FormatFilter(format))
	{
		OutputOffsetProfile::OutputOffset offsetValue =
			profile->isCurrentWindowsAudioFormat
			? profile->GetOffsetForCurrentWinFormat()
			: profile->GetOffsetForFormat(format);
		std::string offsetStr = std::format("{}: {} ms", audioFormatStr, offsetValue.value);
		if (offsetValue.verified)
		{
			profile->VerifiedOffsetsForDisplay.push_back(offsetStr);
		}
		else
		{
			profile->UnverifiedOffsetsForDisplay.push_back(offsetStr);
		}
	}
}

OutputOffsetProfile::OutputOffset OutputOffsetProfiles::None_GetOffset(int numChannels, int sampleRate, int bitDepth)
{
	OutputOffsetProfile::OutputOffset result;
	return result;
}

OutputOffsetProfile::OutputOffset OutputOffsetProfiles::None_GetOffset(FileAudioFormats::FileId id)
{
	OutputOffsetProfile::OutputOffset result;
	return result;
}

OutputOffsetProfile::OutputOffset OutputOffsetProfiles::Hdmi_HDV_MB01_GetOffset(int numChannels, int sampleRate, int bitDepth)
{
	OutputOffsetProfile::OutputOffset result;

	float v44100 = -0.1197f;
	float v48000 = -0.1030f;
	float v96000 = -0.0100f;
	float v192000 = 0.0181f;

	if (sampleRate == 44100) { result.SetValue(v44100); }
	else if (sampleRate == 48000) { result.SetValue(v48000); }
	else if (sampleRate == 96000) { result.SetValue(v96000); }
	else if (sampleRate == 192000) { result.SetValue(v192000); }
	else if (sampleRate < 44100) { result.value = v44100; result.verified = false; }
	else if (sampleRate > 44100 && sampleRate < 48000) { result.value = (v44100 + v48000) / 2; result.verified = false; }
	else if (sampleRate > 48000 && sampleRate < 96000) { result.value = (v48000 + v96000) / 2; result.verified = false; }
	else if (sampleRate > 96000 && sampleRate < 192000) { result.value = (v96000 + v192000) / 2; result.verified = false; }
	else if (sampleRate > 192000) { result.value = v192000; result.verified = false; }
	else {
#ifdef _DEBUG
		throw "I wrote this logic wrong. This code should never trigger.";
#else
		result.value = v48000;
		result.verified = false;
#endif
	}

	return result;
}

bool OutputOffsetProfiles::AllFormatsFilter(AudioFormat* format)
{
	return true;
}

bool OutputOffsetProfiles::HdmiAllFormatsFilter(AudioFormat* format, bool onlyLpcm)
{
	bool result = true;
	WAVEFORMATEX* waveFormat = format->GetWaveFormat();

	if (format->type == AudioFormat::FormatType::File)
	{
		if (onlyLpcm)
		{
			result = false;
		}
		else
		{
			// HDMI is able to handle all the file formats included in this toolkit
		}
	}
	else if (AudioFormat::GetFormatID(waveFormat) == WAVE_FORMAT_PCM)
	{
		// Exclude mono because my HDMI signal analyzer gets all types of confused with a "mono" signal
		// which suggests that it's not a valid HDMI format, at least when prepared by NVIDIA HDMI audio drivers.
		if (waveFormat->nChannels < 2)
		{
			result = false;
		}

		if (waveFormat->wBitsPerSample != 16
			&& waveFormat->wBitsPerSample != 20 // This hasn't been tested because the wave formats list doesn't include 20 bit samples yet.
			&& waveFormat->wBitsPerSample != 24)
		{
			result = false;
		}

		if (waveFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
		{
			auto waveFormatExtensible = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(waveFormat);

			// For these two channel masks, Windows HDMI drivers (NVIDIA and Intel at least) replace SPEAKER_BACK (RLC and RRC)
			// with SPEAKER_SIDE (RL and RR), which is a format that is already included in the wave formats list, so this would
			// end up just being an incorrectly displayed duplicate.
			if (waveFormatExtensible->dwChannelMask == KSAUDIO_SPEAKER_QUAD
				|| waveFormatExtensible->dwChannelMask == KSAUDIO_SPEAKER_5POINT1)
			{
				result = false;
			}
		}
	}
	else if (AudioFormat::GetFormatID(waveFormat) == WAVE_FORMAT_DOLBY_AC3_SPDIF) // A.K.A KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_DIGITAL_PLUS
	{
		if (onlyLpcm)
		{
			result = false;
		}
		else
		{
			// keep result == true because all configurations provided by WindowsWaveFormats class are valid over HDMI
		}
	}
	else
	{
		// Unknown if other formats could be supported by HDMI. Either way, WasapiOutput currently does not support them (As of Aug 28, 2023)
		result = false;
	}

	return result;
}

bool OutputOffsetProfiles::HdmiAllFormatsFilter(AudioFormat* format)
{
	return HdmiAllFormatsFilter(format, false);
}

bool OutputOffsetProfiles::HdmiLpcmFormatsFilter(AudioFormat* format)
{
	return HdmiAllFormatsFilter(format, true);
}

bool OutputOffsetProfiles::SpdifAllFormatFilter(AudioFormat* format, bool onlyLpcm)
{
	bool result = true;
	WAVEFORMATEX* waveFormat = format->GetWaveFormat();

	if (format->type == AudioFormat::FormatType::File)
	{
		if (onlyLpcm)
		{
			result = false;
		}
		else
		{
			// I guess this is OK? TODO: Better filtering based on what S/PDIF can actually handle
		}
	}
	else if (AudioFormat::GetFormatID(format->GetWaveFormat()) == WAVE_FORMAT_PCM)
	{
		if (waveFormat->nChannels != 2)
		{
			result = false;
		}

		if (waveFormat->wBitsPerSample != 16
			&& waveFormat->wBitsPerSample != 20 // This hasn't been tested because the wave formats list doesn't include 20 bit samples yet.
			&& waveFormat->wBitsPerSample != 24)
		{
			result = false;
		}
	}
	else if (AudioFormat::GetFormatID(waveFormat) == WAVE_FORMAT_DOLBY_AC3_SPDIF) // A.K.A KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_DIGITAL_PLUS
	{
		if (onlyLpcm)
		{
			result = false;
		}
		else
		{
			// keep result == true because all configurations provided by WindowsWaveFormats class are valid over S/PDIF
		}
	}
	else
	{
		// Unknown if other formats could be supported by S/PDIF. Either way, WasapiOutput currently does not support them (As of Aug 28, 2023)
		result = false;
	}

	return result;
}

bool OutputOffsetProfiles::SpdifAllFormatFilter(AudioFormat* format)
{
	return SpdifAllFormatFilter(format, false);
}

bool OutputOffsetProfiles::SpdifLpcmFormatFilter(AudioFormat* format)
{
	return SpdifAllFormatFilter(format, true);
}

OutputOffsetProfile::OutputOffset OutputOffsetProfiles::Spdif_HDV_MB01_GetOffset(int numChannels, int sampleRate, int bitDepth)
{
	OutputOffsetProfile::OutputOffset result;

	if (numChannels == 2 && sampleRate == 44100 && (bitDepth == 16 || bitDepth == 24)) { result.SetValue(-0.2f); }
	else if (numChannels == 2 && sampleRate == 48000 && (bitDepth == 16 || bitDepth == 24)) { result.SetValue(-0.2f); }
	else if (numChannels == 2 && sampleRate == 96000 && (bitDepth == 16 || bitDepth == 24)) { result.SetValue(-0.1f); }
	else if (numChannels == 2 && sampleRate == 192000 && (bitDepth == 16 || bitDepth == 24)) { result.SetValue(0); }
	else { result.value = -0.1f; result.verified = false; } // Default to median offset

	return result;
}

OutputOffsetProfile::OutputOffset OutputOffsetProfiles::Spdif_AYSA11_GetOffset(int numChannels, int sampleRate, int bitDepth)
{
	OutputOffsetProfile::OutputOffset result;

	if (numChannels == 2 && sampleRate == 44100 && (bitDepth == 16 || bitDepth == 24)) { result.SetValue(-0.1f); }
	else if (numChannels == 2 && sampleRate == 48000 && (bitDepth == 16 || bitDepth == 24)) { result.SetValue(-0.1f); }
	else if (numChannels == 2 && sampleRate == 96000 && (bitDepth == 16 || bitDepth == 24)) { result.SetValue(0); }
	else if (numChannels == 2 && sampleRate == 192000 && (bitDepth == 16 || bitDepth == 24)) { result.SetValue(0); }
	else { result.value = 0; result.verified = false; } // Default to median offset

	return result;
}

OutputOffsetProfile::OutputOffset OutputOffsetProfiles::Spdif_LiNKFOR_USB_DAC_GetOffset(int numChannels, int sampleRate, int bitDepth)
{
	OutputOffsetProfile::OutputOffset result;

	if (numChannels == 2 && sampleRate == 48000 && bitDepth == 16) { result.SetValue(0); }
	else if (numChannels == 2 && sampleRate == 44100 && bitDepth == 16) { result.SetValue(0); }
	else { result.value = 0; result.verified = false; }

	return result;
}

OutputOffsetProfile::OutputOffset OutputOffsetProfiles::Analog_BasicYSplitter_GetOffset(int numChannels, int sampleRate, int bitDepth)
{
	OutputOffsetProfile::OutputOffset result;
	result.SetValue(0);
	return result;
}

OutputOffsetProfile::OutputOffset OutputOffsetProfiles::Arc_ExampleReferenceSetup_GetOffset(int numChannels, int sampleRate, int bitDepth)
{
	OutputOffsetProfile::OutputOffset result;
	// TODO: Set the value based on the parameters of this function and the actual measured offset of the device!
	result.SetValue(0.5);
	return result;
}

OutputOffsetProfile::OutputOffset OutputOffsetProfiles::EArc_ExampleReferenceSetup_GetOffset(int numChannels, int sampleRate, int bitDepth)
{
	OutputOffsetProfile::OutputOffset result;
	// TODO: Set the value based on the parameters of this function and the actual measured offset of the device!
	result.SetValue(0.2);
	return result;
}
