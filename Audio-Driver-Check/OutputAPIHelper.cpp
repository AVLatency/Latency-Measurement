#include "OutputAPIHelper.h"

std::string OutputAPIHelper::OuptutTypeString(OutputAPI type)
{
	switch (type)
	{
	case OutputAPI::WasapiExclusive:
		return "Windows Audio Exclusive Mode (WASAPI)";
		break;
	case OutputAPI::AudioGraph:
		return "Current Windows Audio Format (Audio Graph API)";
		break;
	case OutputAPI::ENUM_LENGTH:
		return "None";
		break;
	default:
		throw "unsupported output type";
	}
}