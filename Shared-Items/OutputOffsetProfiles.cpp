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

std::vector<OutputOffsetProfile*> OutputOffsetProfiles::Profiles;
int OutputOffsetProfiles::SelectedProfileIndex = 0;

std::map<OutputOffsetProfile::OutputType, OutputOffsetProfiles::ProfilesSubset*> OutputOffsetProfiles::Subsets;

void OutputOffsetProfiles::InitializeProfiles(ProfileResources& resources)
{
	// *****************************************************
	// HDMI AUDIO LATENCY PROFILES
	// *****************************************************

	Hdmi_HDV_MB01 = new OutputOffsetProfile(OutputOffsetProfile::OutputType::Hdmi, "HDV-MB01", Hdmi_HDV_MB01_GetOffset, AudioEndpoint::HdmiFormatsFilter);
	Hdmi_HDV_MB01->Image = resources.Hdmi_HDV_MB01_Texture;
	Hdmi_HDV_MB01->Description =
		"The HDV-MB01 is sold under these names:\n\n"
		
		"- J-Tech Digital JTD18G - H5CH\n"
		"- Monoprice Blackbird 24278\n"
		"- OREI HDA - 912";
	Profiles.push_back(Hdmi_HDV_MB01);

	Hdmi_None = new OutputOffsetProfile(OutputOffsetProfile::OutputType::Hdmi, "Other", None_GetOffset, AudioEndpoint::HdmiFormatsFilter);
	Hdmi_None->isNoOffset = true;
	Profiles.push_back(Hdmi_None);

	Hdmi_None_CurrentWinAudioFormat = new OutputOffsetProfile(OutputOffsetProfile::OutputType::Hdmi, "Other (Current Windows audio format)", None_GetOffset, AudioEndpoint::HdmiFormatsFilter);
	Hdmi_None_CurrentWinAudioFormat->isNoOffset = true;
	Hdmi_None_CurrentWinAudioFormat->isCurrentWindowsAudioFormat = true;
	Profiles.push_back(Hdmi_None_CurrentWinAudioFormat);

	// *****************************************************
	// S/PDIF AUDIO LATENCY PROFILES
	// *****************************************************

	Spdif_HDV_MB01 = new OutputOffsetProfile(OutputOffsetProfile::OutputType::Spdif, "HDV-MB01", Spdif_HDV_MB01_GetOffset, CommonSpdifFormatFilter);
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

	Spdif_AYSA11 = new OutputOffsetProfile(OutputOffsetProfile::OutputType::Spdif, "AYSA11", Spdif_AYSA11_GetOffset, CommonSpdifFormatFilter);
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

	Spdif_LiNKFOR_USB_DAC = new OutputOffsetProfile(OutputOffsetProfile::OutputType::Spdif, "LiNKFOR USB DAC", Spdif_LiNKFOR_USB_DAC_GetOffset, CommonSpdifFormatFilter);
	Spdif_LiNKFOR_USB_DAC->Description =
		"Supported S/PDIF Formats:\n\n"

		"2ch-44.1kHz-16bit\n"
		"2ch-48kHz-16bit\n";
	Profiles.push_back(Spdif_LiNKFOR_USB_DAC);

	Spdif_None = new OutputOffsetProfile(OutputOffsetProfile::OutputType::Spdif, "Other", None_GetOffset, AudioEndpoint::AllFormatsFilter);
	Spdif_None->isNoOffset = true;
	Profiles.push_back(Spdif_None);

	Spdif_None_CurrentWinAudioFormat = new OutputOffsetProfile(OutputOffsetProfile::OutputType::Spdif, "Other (Current Windows audio format)", None_GetOffset, AudioEndpoint::AllFormatsFilter);
	Spdif_None_CurrentWinAudioFormat->isNoOffset = true;
	Spdif_None_CurrentWinAudioFormat->isCurrentWindowsAudioFormat = true;
	Profiles.push_back(Spdif_None_CurrentWinAudioFormat);

	// *****************************************************
	// ARC AUDIO LATENCY PROFILES
	// *****************************************************

#ifdef _DEBUG
	Arc_ExampleReferenceSetup = new OutputOffsetProfile(OutputOffsetProfile::OutputType::ARC, "Example ARC Reference Setup", Arc_ExampleReferenceSetup_GetOffset, AudioEndpoint::AllFormatsFilter);
	Arc_ExampleReferenceSetup->Image = resources.Arc_ExampleDevice_Texture;
	Arc_ExampleReferenceSetup->Description =
		"This is a DEBUG-only example ARC Receiver Latency reference device/setup that can be used as a starting point for creating new Output Offset Profiles.";
	Profiles.push_back(Arc_ExampleReferenceSetup);
#endif //_DEBUG

	Arc_None = new OutputOffsetProfile(OutputOffsetProfile::OutputType::ARC, "Other", None_GetOffset, AudioEndpoint::AllFormatsFilter);
	Arc_None->isNoOffset = true;
	Profiles.push_back(Arc_None);

	Arc_None_CurrentWinAudioFormat = new OutputOffsetProfile(OutputOffsetProfile::OutputType::ARC, "Other (Current Windows audio format)", None_GetOffset, AudioEndpoint::AllFormatsFilter);
	Arc_None_CurrentWinAudioFormat->isNoOffset = true;
	Arc_None_CurrentWinAudioFormat->isCurrentWindowsAudioFormat = true;
	Profiles.push_back(Arc_None_CurrentWinAudioFormat);

	// *****************************************************
	// EARC AUDIO LATENCY PROFILES
	// *****************************************************

#ifdef _DEBUG
	EArc_ExampleReferenceSetup = new OutputOffsetProfile(OutputOffsetProfile::OutputType::eARC, "Example eARC Reference Setup", EArc_ExampleReferenceSetup_GetOffset, AudioEndpoint::AllFormatsFilter);
	EArc_ExampleReferenceSetup->Image = resources.EArc_ExampleDevice_Texture;
	EArc_ExampleReferenceSetup->Description =
		"This is a DEBUG-only example eARC Receiver Latency reference device/setup that can be used as a starting point for creating new Output Offset Profiles.";
	Profiles.push_back(EArc_ExampleReferenceSetup);
#endif //_DEBUG

	EArc_None = new OutputOffsetProfile(OutputOffsetProfile::OutputType::eARC, "Other", None_GetOffset, AudioEndpoint::AllFormatsFilter);
	EArc_None->isNoOffset = true;
	Profiles.push_back(EArc_None);

	EArc_None_CurrentWinAudioFormat = new OutputOffsetProfile(OutputOffsetProfile::OutputType::eARC, "Other (Current Windows audio format)", None_GetOffset, AudioEndpoint::AllFormatsFilter);
	EArc_None_CurrentWinAudioFormat->isNoOffset = true;
	EArc_None_CurrentWinAudioFormat->isCurrentWindowsAudioFormat = true;
	Profiles.push_back(EArc_None_CurrentWinAudioFormat);

	// *****************************************************
	// ANALOG AUDIO LATENCY PROFILES
	// *****************************************************

	Analog_BasicYSplitter = new OutputOffsetProfile(OutputOffsetProfile::OutputType::Analog, "Basic Analog Y Splitter", Analog_BasicYSplitter_GetOffset, AudioEndpoint::AllFormatsFilter);
	Analog_BasicYSplitter->Image = resources.Analog_YSplitter_Texture;
	Analog_BasicYSplitter->Description =
		"For measuring analog audio latency, a basic analog Y splitter can be used to produce "
		"two identical outputs that can act as a \"Dual-Out Reference\".";
	Profiles.push_back(Analog_BasicYSplitter);

	Analog_None = new OutputOffsetProfile(OutputOffsetProfile::OutputType::Analog, "Other", None_GetOffset, AudioEndpoint::AllFormatsFilter);
	Analog_None->isNoOffset = true;
	Profiles.push_back(Analog_None);

	// *****************************************************
	// HDMI AUDIO PASSTHROUGH LATENCY PROFILES
	// *****************************************************

	// A copy of the Hdmi_HDV_MB01 profile, but with OutputType::HdmiAudioPassthrough
	HdmiAudioPassthrough_HDV_MB01 = new OutputOffsetProfile(OutputOffsetProfile::OutputType::HdmiAudioPassthrough, "HDV-MB01", Hdmi_HDV_MB01_GetOffset, AudioEndpoint::HdmiFormatsFilter);
	HdmiAudioPassthrough_HDV_MB01->Image = Hdmi_HDV_MB01->Image;
	HdmiAudioPassthrough_HDV_MB01->Description = Hdmi_HDV_MB01->Description;
	Profiles.push_back(HdmiAudioPassthrough_HDV_MB01);

	HdmiAudioPassthrough_None = new OutputOffsetProfile(OutputOffsetProfile::OutputType::HdmiAudioPassthrough, "Other", None_GetOffset, AudioEndpoint::HdmiFormatsFilter);
	HdmiAudioPassthrough_None->isNoOffset = true;
	Profiles.push_back(HdmiAudioPassthrough_None);

	HdmiAudioPassthrough_None_CurrentWinAudioFormat = new OutputOffsetProfile(OutputOffsetProfile::OutputType::HdmiAudioPassthrough, "Other (Current Windows audio format)", None_GetOffset, AudioEndpoint::HdmiFormatsFilter);
	HdmiAudioPassthrough_None_CurrentWinAudioFormat->isNoOffset = true;
	HdmiAudioPassthrough_None_CurrentWinAudioFormat->isCurrentWindowsAudioFormat = true;
	Profiles.push_back(HdmiAudioPassthrough_None_CurrentWinAudioFormat);

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
	std::stringstream ss;
	for (int i = 0; i < Profiles.size(); i++)
	{
		OutputOffsetProfile::OutputOffset offset = Profiles[i]->GetOffset(2, 48000, 16);
		if (offset.verified)
		{
			std::string highlightedOffsetValue = std::format("LPCM 2ch-48kHz-16bit: {} ms", offset.value);
			Profiles[i]->HighlightedVerifiedOffsetsForDisplay.push_back(highlightedOffsetValue);
		}
		offset = Profiles[i]->GetOffset(2, 192000, 16);
		if (offset.verified)
		{
			std::string highlightedOffsetValue = std::format("LPCM 2ch-192kHz-16bit: {} ms", offset.value);
			Profiles[i]->HighlightedVerifiedOffsetsForDisplay.push_back(highlightedOffsetValue);
		}

		offset = Profiles[i]->GetOffset(6, 48000, 16);
		if (offset.verified)
		{
			std::string highlightedOffsetValue = std::format("LPCM 6ch-48kHz-16bit: {} ms", offset.value);
			Profiles[i]->HighlightedVerifiedOffsetsForDisplay.push_back(highlightedOffsetValue);
		}
		offset = Profiles[i]->GetOffset(6, 192000, 16);
		if (offset.verified)
		{
			std::string highlightedOffsetValue = std::format("LPCM 6ch-192kHz-16bit: {} ms", offset.value);
			Profiles[i]->HighlightedVerifiedOffsetsForDisplay.push_back(highlightedOffsetValue);
		}

		offset = Profiles[i]->GetOffset(6, 48000, 16);
		if (offset.verified)
		{
			std::string highlightedOffsetValue = std::format("LPCM 8ch-48kHz-16bit: {} ms", offset.value);
			Profiles[i]->HighlightedVerifiedOffsetsForDisplay.push_back(highlightedOffsetValue);
		}
		offset = Profiles[i]->GetOffset(6, 192000, 16);
		if (offset.verified)
		{
			std::string highlightedOffsetValue = std::format("LPCM 8ch-192kHz-16bit: {} ms", offset.value);
			Profiles[i]->HighlightedVerifiedOffsetsForDisplay.push_back(highlightedOffsetValue);
		}

		for (int f = 0; f < WindowsWaveFormats::Formats.AllExFormats.size(); f++)
		{
			WAVEFORMATEX* format = WindowsWaveFormats::Formats.AllExFormats[f];
			if (format->wFormatTag == WAVE_FORMAT_PCM)
			{
				OutputOffsetProfile::OutputOffset offsetValue = Profiles[i]->GetOffsetFromWaveFormat(format);
				ss << AudioFormat::GetFormatString(WindowsWaveFormats::Formats.AllExFormats[f], true, false) << ": ";
				if (offsetValue.verified)
				{
					ss << offsetValue.value << " ms";;
					Profiles[i]->VerifiedOffsetsForDisplay.push_back(ss.str());
				}
				else
				{
					ss << offsetValue.value << " ms";;
					Profiles[i]->UnverifiedOffsetsForDisplay.push_back(ss.str());
				}
				ss.str(std::string());
			}
		}
	}
}

OutputOffsetProfile::OutputOffset OutputOffsetProfiles::None_GetOffset(int numChannels, int sampleRate, int bitDepth)
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

bool OutputOffsetProfiles::CommonSpdifFormatFilter(WAVEFORMATEX* waveFormat)
{
	bool result = true;

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

	return result;
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
