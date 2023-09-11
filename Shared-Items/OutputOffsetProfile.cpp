#include "OutputOffsetProfile.h"

void OutputOffsetProfile::OutputOffset::SetValue(float value)
{
	this->value = value;
	verified = true;
}

std::string OutputOffsetProfile::OutputTypeName(OutputOffsetProfile::OutputType outputType)
{
	switch (outputType)
	{
	case OutputOffsetProfile::OutputType::None:
		return "";
		break;
	case OutputOffsetProfile::OutputType::Hdmi:
		return "HDMI";
		break;
	case OutputOffsetProfile::OutputType::Spdif:
		return "S/PDIF";
		break;
	case OutputOffsetProfile::OutputType::ARC:
		return "ARC Receiver";
		break;
	case OutputOffsetProfile::OutputType::eARC:
		return "eARC Receiver";
		break;
	case OutputOffsetProfile::OutputType::Analog:
		return "Analog";
		break;
	case OutputOffsetProfile::OutputType::HdmiAudioPassthrough:
		return "HDMI Audio Passthrough";
		break;
	case OutputOffsetProfile::OutputType::RelativeWinAudio:
		return "Relative Windows Audio";
		break;
	default:
		throw "Attempting to get OutputTypeName for an OutputType that has not been handled.";
	}
}

std::string OutputOffsetProfile::OutputTypeNameFileSafe(OutputOffsetProfile::OutputType outputType)
{
	switch (outputType)
	{
	case OutputOffsetProfile::OutputType::None:
		return " ";
		break;
	case OutputOffsetProfile::OutputType::Hdmi:
		return "HDMI";
		break;
	case OutputOffsetProfile::OutputType::Spdif:
		return "SPDIF";
		break;
	case OutputOffsetProfile::OutputType::ARC:
		return "ARC RX";
		break;
	case OutputOffsetProfile::OutputType::eARC:
		return "eARC RX";
		break;
	case OutputOffsetProfile::OutputType::Analog:
		return "Analog";
		break;
	case OutputOffsetProfile::OutputType::HdmiAudioPassthrough:
		return "HDMI Passthrgh";
		break;
	case OutputOffsetProfile::OutputType::RelativeWinAudio:
		return "Relative Win";
		break;
	default:
		throw "Attempting to get OutputTypeNameFileSafe for an OutputType that has not been handled.";
	}
}

OutputOffsetProfile::OutputOffsetProfile(OutputType type, std::string name, OutputOffset(*getLpcmOffsetFunc)(int numChannels, int sampleRate, int bitDepth), OutputOffset(*getFileOffsetFunc)(FileAudioFormats::FileId id), bool (*formatFilter)(AudioFormat*))
	: OutType(type), Name(name), GetLpcmOffset(getLpcmOffsetFunc), GetFileOffset(getFileOffsetFunc), FormatFilter(formatFilter), Image(AVLTexture()), isNoOffset(false), isCurrentWindowsAudioFormat(false)
{
}

OutputOffsetProfile::OutputOffset OutputOffsetProfile::GetOffsetForFormat(AudioFormat* format)
{
	if (format->type == AudioFormat::FormatType::WaveFormatEx)
	{
		return GetOffsetForWaveFormat(format->GetWaveFormat());
	}
	else
	{
		return GetFileOffset(format->FileId);
	}
}

OutputOffsetProfile::OutputOffset OutputOffsetProfile::GetOffsetForWaveFormat(WAVEFORMATEX* waveFormat)
{
	return GetLpcmOffset(waveFormat->nChannels, waveFormat->nSamplesPerSec, waveFormat->wBitsPerSample);
}

OutputOffsetProfile::OutputOffset OutputOffsetProfile::GetOffsetForCurrentWinFormat()
{
	return GetLpcmOffset(0, 0, 0);
}