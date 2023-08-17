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
	default:
		throw "Attempting to get OutputTypeNameFileSafe for an OutputType that has not been handled.";
	}
}

OutputOffsetProfile::OutputOffsetProfile(OutputType type, std::string name, OutputOffset(*getOffsetFunc)(int numChannels, int sampleRate, int bitDepth), bool (*formatFilter)(WAVEFORMATEX*))
	: OutType(type), Name(name), GetOffset(getOffsetFunc), FormatFilter(formatFilter), Image(AVLTexture()), isNoOffset(false), isCurrentWindowsAudioFormat(false)
{
}

OutputOffsetProfile::OutputOffset OutputOffsetProfile::GetOffsetFromWaveFormat(WAVEFORMATEX* waveFormat)
{
	return GetOffset(waveFormat->nChannels, waveFormat->nSamplesPerSec, waveFormat->wBitsPerSample);
}

OutputOffsetProfile::OutputOffset OutputOffsetProfile::GetOffsetForCurrentWinFormat()
{
	return GetOffset(0, 0, 0);
}