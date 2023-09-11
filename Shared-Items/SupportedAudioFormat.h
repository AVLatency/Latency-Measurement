#pragma once
#include "AudioFormat.h"

class AudioFormat;

class SupportedAudioFormat
{
public:
	SupportedAudioFormat(AudioFormat* format) : Format(format) { };

	AudioFormat* Format;
	bool UserSelected = false;
	bool DefaultSelection = false;
};
